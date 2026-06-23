#include "Cubed/gameplay/server_world.hpp"

#include "Cubed/config.hpp"
#include "Cubed/gameplay/packet.hpp"
#include "Cubed/gameplay/session.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <utility>
using namespace std::chrono;
using namespace std::chrono_literals;

namespace Cubed {
ServerWorld::ServerWorld() {}

ServerWorld::~ServerWorld() {
    stop_gen_thread();
    stop_server_thread();
    wait_all_chunk_tasks();
    stop_thread_pool();

    m_chunks.clear();
}

void ServerWorld::wait_all_chunk_tasks() {
    for (auto& [pos, task] : new_chunks) {
        task.future.get();
    }
}

void ServerWorld::init_world() {
    m_cave_carcer.init(ChunkGenerator::seed());
    m_river_worm.init(ChunkGenerator::seed());
    m_chunks.reserve(MAX_DISTANCE * MAX_DISTANCE * 4);
    start_thread_pool();

    auto t1 = std::chrono::system_clock::now();

    // init players
    // m_players.emplace(HASH::str("TestPlayer"), Player(*this, "TestPlayer"));

    start_gen_thread();
    init_chunks();
    auto t2 = std::chrono::system_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    Logger::info("Chunk Block Init Finish, Time Consuming: {}", d);

    start_server_thread();

    Logger::info("TestPlayer Create Finish");
}

void ServerWorld::init_chunks() {
    hot_reload();
    while (!m_chunk_gen_finished) {
        // Logger::info("World Spawn: {:.2f}%", m_chunk_gen_fraction.load());
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
}

void ServerWorld::gen_chunks_internal() {
    // Logger::info("gen_chunks_internal");
    m_chunk_gen_finished = false;

    ChunkPosSet required_chunks;
    compute_required_chunks(required_chunks);

    ASSERT_MSG(!required_chunks.empty(), "required chunks is empty!!");

    std::vector<ChunkPos> need_gen_chunks_pos;

    sync_and_collect_missing_chunks(need_gen_chunks_pos, required_chunks);

    Logger::info("New Gen Chunks Sum: {}", need_gen_chunks_pos.size());

    if (need_gen_chunks_pos.empty()) {
        m_could_gen = true;

        return;
    }

    for (auto& pos : need_gen_chunks_pos) {
        new_chunks.emplace(pos, ServerChunk(*this, pos));
    }

    submit_new_chunks();
    m_chunk_gen_finished = true;
    m_request_gen_name = std::nullopt;
}

void ServerWorld::compute_required_chunks(ChunkPosSet& required_chunks) {
    glm::vec3 player_pos;
    if (m_request_gen_name == std::nullopt) {
        player_pos = glm::vec3{0.0f};
    } else {
        player_pos = get_player_pos(m_request_gen_name.value());
    }
    int x = std::floor(player_pos.x);
    int z = std::floor(player_pos.z);
    auto [chunk_x, chunk_z] = get_chunk_pos(x, z);
    int radius = m_rendering_distance;
    int r2 = radius * radius;
    required_chunks.reserve(radius * radius);

    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            if (dx * dx + dz * dz <= r2) {
                required_chunks.emplace(chunk_x + dx, chunk_z + dz);
            }
        }
    }
}
void ServerWorld::sync_and_collect_missing_chunks(
    std::vector<ChunkPos>& need_gen_chunks_pos,
    const ChunkPosSet& required_chunks) {
    std::lock_guard lk(m_chunks_mutex);
    for (auto it = m_chunks.begin(); it != m_chunks.end();) {
        if (required_chunks.find(it->first) == required_chunks.end()) {
            it = m_chunks.erase(it);
        } else {
            ++it;
        }
    }

    for (auto pos : required_chunks) {
        auto it = m_chunks.find(pos);
        if (it == m_chunks.end()) {
            need_gen_chunks_pos.push_back(pos);
        }
    }
}
void ServerWorld::submit_new_chunks() {
    using enum ChunkLoadStyle;
    std::lock_guard lock(m_new_chunk_mutex);
    auto pool_ptr = m_gen_thread_pool.load();
    if (!pool_ptr) {
        return;
    }
    switch (m_chunk_load_style) {
    case RANDOM:
        for (auto& [pos, task] : new_chunks) {
            if (!task.future.valid()) {
                task.future =
                    pool_ptr->enqueue([&task]() { task.chunk.gen_chunk(); });
            }
        }
        break;
    case CENTER: {
        std::vector<std::pair<ChunkPos, PendingChunk*>> tasks;
        for (auto& [pos, task] : new_chunks) {
            if (!task.future.valid()) {
                tasks.emplace_back(pos, &task);
            }
        }
        glm::vec3 player_pos;
        if (m_request_gen_name == std::nullopt) {
            player_pos = glm::vec3{0.0f};
        } else {
            player_pos = get_player_pos(m_request_gen_name.value());
        }
        auto dist2 = [player_pos](ChunkPos chunk_pos) {
            ChunkPos player_chunk_pos =
                get_chunk_pos(player_pos.x, player_pos.z);
            float dx = player_chunk_pos.x - chunk_pos.x;
            float dz = player_chunk_pos.z - chunk_pos.z;
            return dx * dx + dz * dz;
        };

        std::sort(tasks.begin(), tasks.end(),
                  [&dist2](const auto& a, const auto& b) {
                      return dist2(a.first) < dist2(b.first);
                  });
        for (auto& [pos, task] : tasks) {
            if (!task->future.valid()) {
                task->future =
                    pool_ptr->enqueue([task]() { task->chunk.gen_chunk(); });
            }
        }
    }
    }
}

void ServerWorld::poll_finished_chunks() {
    m_new_finished_chunk.clear();
    std::lock_guard lock(m_new_chunk_mutex);
    std::erase_if(
        new_chunks, [&](std::pair<const ChunkPos, PendingChunk>& pair) {
            auto& pending = pair.second;
            if (!pending.future.valid()) {
                return false;
            }
            if (pending.future.wait_for(0ms) != std::future_status::ready) {
                return false;
            }
            pending.future.get();

            m_new_finished_chunk.emplace_back(pair.first,
                                              std::move(pending.chunk));
            return true;
        });
}

void ServerWorld::start_gen_thread() {
    m_gen_running = true;
    Logger::info("Gen Thread Started");
    m_gen_thread = std::thread([this]() {
        while (m_gen_running) {
            std::unique_lock<std::mutex> lk(m_gen_signal_mutex);

            m_gen_cv.wait(lk, [this]() {
                return m_need_gen_chunk.load() || !m_gen_running;
            });
            if (!m_gen_running) {
                break;
            }
            m_need_gen_chunk = false;
            lk.unlock();

            gen_chunks_internal();
        }
    });
}

void ServerWorld::start_server_thread() {
    m_server_thread = std::thread(
        [this]() { serever_run(m_server_stop_source.get_token()); });
}

void ServerWorld::start_thread_pool() {
    int max_thread = std::thread::hardware_concurrency();
    if (m_pool_threads == 0) {
        change_pool_threads(max_thread - RESERVED_THREADS);
    } else {
        change_pool_threads(m_pool_threads);
    }
}

void ServerWorld::stop_gen_thread() {
    m_gen_running = false;
    m_gen_cv.notify_all();
    if (m_gen_thread.joinable()) {
        m_gen_thread.join();
    }
    Logger::info("Gen Thread Stopped");
}

void ServerWorld::stop_server_thread() {
    m_server_stop_source.request_stop();
    if (m_server_thread.joinable()) {
        m_server_thread.join();
    }
}

void ServerWorld::stop_thread_pool() {
    auto pool_ptr = m_gen_thread_pool.load();
    if (pool_ptr) {
        pool_ptr->stop();
    }
    m_gen_thread_pool.store(nullptr);
    Logger::info("Thread Pool Stopped");
}

void ServerWorld::serever_run(std::stop_token stoken) {
    Logger::info("Server Thread Started!");
    while (!stoken.stop_requested()) {
        std::this_thread::sleep_for(milliseconds(m_per_tick_time));
        if (m_tick_running) {
            ++m_game_ticks;
            m_day_tick = (m_day_tick + 1) % DAY_TIME;
        }
        update();
    }
    Logger::info("Server Thread Stopped!");
}

void ServerWorld::need_gen() {

    if (!m_could_gen) {
        Logger::warn("It is generating or consuming new chunks");
        return;
    }

    m_could_gen = false;
    {
        // std::lock_guard lk(m_gen_player_pos_mutex);
        // m_gen_player_pos = get_player("TestPlayer").get_player_pos();
        ASSERT_MSG(false, "Player Pos Handle");
    }

    m_need_gen_chunk = true;

    m_gen_cv.notify_one();
}

void ServerWorld::set_block(const glm::ivec3& block_pos, unsigned id) {

    int world_x, world_y, world_z;
    world_x = block_pos.x;
    world_y = block_pos.y;
    world_z = block_pos.z;

    auto [chunk_x, chunk_z] = get_chunk_pos(world_x, world_z);
    std::lock_guard lk(m_chunks_mutex);
    auto it = m_chunks.find(ChunkPos{chunk_x, chunk_z});

    if (it == m_chunks.end()) {
        return;
    }

    auto [x, y, z] = ServerChunk::world_to_block(world_x, world_y, world_z,
                                                 chunk_x, chunk_z);
    if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
        z >= CHUNK_SIZE) {
        return;
    }

    it->second.set_chunk_block(ServerChunk::index(x, y, z), id);
}

void ServerWorld::hot_reload() {
    auto& config = Config::get();
    int dist = config.get<int>("world.rendering_distance");
    m_rendering_distance = dist <= MAX_DISTANCE ? dist : MAX_DISTANCE;
    need_gen();
}

void ServerWorld::rebuild_world() {
    if (m_is_rebuilding) {
        return;
    }
    m_is_rebuilding = true;
    stop_gen_thread();
    stop_thread_pool();
    m_cave_carcer.reload(ChunkGenerator::seed());
    m_river_worm.reload(ChunkGenerator::seed());
    {
        std::lock_guard lk(m_chunks_mutex);
        m_chunks.clear();
        m_new_finished_chunk.clear();
    }
    m_could_gen = true;
    ChunkGenerator::reload();
    start_thread_pool();
    start_gen_thread();
    need_gen();

    m_is_rebuilding = false;
}

void ServerWorld::update() { poll_finished_chunks(); }

void ServerWorld::sync_player_pos(const std::string& name, float x, float y,
                                  float z) {
    auto it = m_players.find(name);
    if (it == m_players.end()) {
        Logger::warn("Player {} is not in this Server", it->first);
        return;
    }
    it->second.update_pos(x, y, z);
}

void ServerWorld::handle_player_login(const std::string& name,
                                      std::shared_ptr<Session> session) {
    player_join(name);
    m_player_session.emplace(name, session);
    LoginRsp rsp;
    rsp.set_success(true);
    rsp.set_uuid(name);
    session->send(make_packet(name));
}

void ServerWorld::player_join(const std::string& name) {
    m_players.emplace(name, name);
}
void ServerWorld::player_exit(const std::string& name) {
    auto it = m_players.find(name);
    if (it == m_players.end()) {
        Logger::error("Player {} isn't in Server", it->first);
    }
    m_players.erase(it);
}

glm::vec3 ServerWorld::get_player_pos(const std::string& name) const {
    auto it = m_players.find(name);
    if (it == m_players.end()) {
        return glm::vec3{0.0f};
    }
    return it->second.get_pos();
}

int ServerWorld::rendering_distance() const {
    return m_rendering_distance.load();
}

void ServerWorld::rendering_distance(int rendering_distance) {
    m_rendering_distance = rendering_distance;
}

CaveCarver& ServerWorld::cave_carcer() { return m_cave_carcer; }
RiverWorm& ServerWorld::river_worm() { return m_river_worm; }

TickType ServerWorld::game_tick() const { return m_game_ticks.load(); }
TickType ServerWorld::day_tick() const { return m_day_tick.load(); }
void ServerWorld::day_tick(TickType tick) {
    tick %= DAY_TIME;
    m_day_tick = tick;
}
int ServerWorld::per_tick_time() const { return m_per_tick_time.load(); }
void ServerWorld::per_tick_time(int ms) { m_per_tick_time = ms; }

bool ServerWorld::is_tick_running() const { return m_tick_running.load(); }
void ServerWorld::tick_running(bool run) { m_tick_running = run; }
int ServerWorld::pool_threads() const { return m_pool_threads.load(); }
int ServerWorld::max_threads() const { return m_max_threads.load(); }
void ServerWorld::change_pool_threads(int threads) {
    m_max_threads = std::thread::hardware_concurrency();
    if (m_max_threads < 1) {
        Logger::warn("Can't Get Max Support Threads, Set Max Threads to 4");
        m_max_threads = 4;
    }
    int used_thread = std::clamp(threads, 1, m_max_threads.load());
    Logger::info("Create New Thread Pool Use {} Threads", used_thread);
    m_gen_thread_pool.store(std::make_shared<ThreadPool>(used_thread));
    m_pool_threads = used_thread;
}
int ServerWorld::chunk_load_style() const {
    return std::to_underlying(m_chunk_load_style.load());
}
void ServerWorld::set_chunk_load_style(int id) {
    using enum ChunkLoadStyle;

    switch (id) {
    case std::to_underlying(RANDOM):
        m_chunk_load_style = RANDOM;
        return;
    case std::to_underlying(CENTER):
        m_chunk_load_style = CENTER;
        return;
    }
    Logger::error("Can,t Find Chunk Load Style Id {}, Nothing Will Do", id);
}

} // namespace Cubed