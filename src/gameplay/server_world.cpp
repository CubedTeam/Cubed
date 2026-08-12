#include "Cubed/gameplay/server_world.hpp"

#include "Cubed/gameplay/packet.hpp"
#include "Cubed/gameplay/session.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/math_tools.hpp"
#include "Cubed/tools/net_utils.hpp"
#include "Cubed/tools/threas_utils.hpp"
#include "Cubed/tools/uuid.hpp"

#include <ranges>
#include <rapidjson/document.h>
#include <tracy/Tracy.hpp>
#include <utility>
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace google::protobuf;
using namespace rapidjson;
namespace fs = std::filesystem;
namespace Cubed {
ServerWorld::ServerWorld(Config& config)
    : m_config(config), m_entity_manager(*this), m_players_manager(*this),
      m_chunk_system(*this) {}

ServerWorld::~ServerWorld() { stop(); }

void ServerWorld::stop() {
    if (!m_init) {
        return;
    }
    if (m_stopped.exchange(true)) {
        return;
    }
    send_server_stop();
    m_chunk_system.stop_gen_thread();
    stop_server_thread();
    // wait_all_chunk_tasks();
    stop_thread_pool();

    m_chunk_system.stop();
}

void ServerWorld::send_time() {
    Arena arena;
    auto* rsp = Arena::Create<UpdateTime>(&arena);

    rsp->set_day_tick(m_day_tick);
    rsp->set_game_tick(m_game_ticks);
    auto sessions = get_all_session();
    auto packet = make_packet(*rsp);
    for (auto& s : sessions) {
        s->send(packet, 3);
    }
}

void ServerWorld::init_world(RunMode mode) {
    m_runmode = mode;
    m_entity_manager.init();
    m_chunk_system.initialize();
    register_timer("player disconnect", 5, [this]() {
        std::vector<std::string> disconnect;
        auto players = m_players_manager.snapshot();
        if (!players) {
            return;
        }
        for (auto& [uuid, player] : *players) {
            if (player->is_disconnect(m_game_ticks)) {
                disconnect.emplace_back(uuid);
            }
        }

        for (auto& uuid : disconnect) {
            handle_player_exit(uuid);
        }
    });
    // Periodically process pending players
    register_timer("player chunk send", 1,
                   [this]() { m_chunk_system.pop_pending_request(); });

    m_cave_carcer.init(ChunkGenerator::seed());
    m_river_worm.init(ChunkGenerator::seed());

    m_enable_filter = m_config.get("sensitive_filter", false);
    Logger::info("sensitive filter {}", m_enable_filter.load());
    m_voice_chat = m_config.get("voice_chat", true);
    Logger::info("voice chat {}", m_voice_chat.load());

    try {
        fs::path path = std::format("{}SensitiveLexicon.json", ASSETS_PATH);
        Document doc;
        if (!Tools::parse_json(doc, path)) {
            throw std::runtime_error("Can't parse SensitiveLexicon.json");
        }
        m_filter.load(doc);
    } catch (const std::exception& e) {
        Logger::error("Load SensitiveLexicon.json Fail");
        m_enable_filter = false;
    }

    // m_chunks.reserve(MAX_DISTANCE * MAX_DISTANCE * 4);
    start_thread_pool();

    m_chunk_system.start_gen_thread();
    init_chunks();

    start_server_thread();
    m_init = true;
}

void ServerWorld::init_chunks() { hot_reload(); }

void ServerWorld::start_server_thread() {
    m_server_thread =
        std::jthread([this](std::stop_token token) { serever_run(token); });
}

void ServerWorld::start_thread_pool() {

    if (m_net_threads == 0) {
        auto net_threads = Tools::get_server_net_pool_threads(m_runmode);
        Logger::info("Server Net pool threads {}", net_threads);
        m_net_threads = change_pool_threads(m_net_thread_pool, net_threads);
    } else {
        m_net_threads = change_pool_threads(m_net_thread_pool, m_net_threads);
    }

    if (m_compute_threads == 0) {
        auto compute_threads = Tools::get_server_compute_treads(m_runmode);
        Logger::info("Server compute pool threads {}", compute_threads);
        m_compute_threads =
            change_pool_threads(m_compute_thread_pool, compute_threads);
    } else {
        m_compute_threads =
            change_pool_threads(m_compute_thread_pool, m_compute_threads);
    }
}

void ServerWorld::stop_server_thread() {
    m_server_thread.request_stop();
    if (m_server_thread.joinable()) {
        m_server_thread.join();
    }
}

void ServerWorld::stop_thread_pool() {

    m_chunk_system.stop_generation_pool();

    auto p = m_net_thread_pool.load();
    if (p) {
        p->stop();
    }
    m_net_thread_pool.store(nullptr);
    Logger::info("Net Thread Pool Stopped");

    auto c = m_compute_thread_pool.load();
    if (c) {
        c->stop();
    }
    m_compute_thread_pool.store(nullptr);
    Logger::info("Compute Thread Pool Stopped");
}

void ServerWorld::serever_run(std::stop_token stoken) {
    Logger::info("Server Thread Started!");
    tracy::SetThreadName("Server Main");
    using Clock = std::chrono::steady_clock;
    const auto TICK = std::chrono::milliseconds(m_per_tick_time);

    auto next = Clock::now();
    while (!stoken.stop_requested()) {

        next += TICK;
        if (m_tick_running) {
            ++m_game_ticks;
            m_day_tick = (m_day_tick + 1) % DAY_TIME;
        }
        update();
        std::this_thread::sleep_until(next);
    }
    Logger::info("Server Thread Stopped!");
}

void ServerWorld::request_generation(std::string uuid) {
    m_chunk_system.request_generation(uuid);
}

bool ServerWorld::set_block(const glm::ivec3& block_pos, BlockType id) {
    return m_chunk_system.set_block(block_pos, id);
}

void ServerWorld::hot_reload() { m_chunk_system.hot_reload(); }

void ServerWorld::update() {
    ZoneScopedN("Server Tick Update");
    // poll_finished_chunks();
    send_time();

    m_entity_manager.update();

    m_chunk_system.update();

    for (auto& [id, timer] : m_timers) {
        timer.update();
    }
}

void ServerWorld::sync_player_pos(const C2S_PlayerInfo& prsp) {
    ZoneScopedN("ServerWorld::sync_player_pos");
    std::string name;
    auto x = prsp.pos().x();
    auto y = prsp.pos().y();
    auto z = prsp.pos().z();
    auto uuid = prsp.uuid();
    auto yaw = prsp.yaw();
    auto pitch = prsp.pitch();

    auto player = m_players_manager.find(uuid);
    if (!player) {
        Logger::warn("Player {} is not in this Server", uuid);
        return;
    }

    player->update_pos(x, y, z);
    player->update_sync_gametick(m_game_ticks);
    player->set_pitch(pitch);
    player->set_yaw(yaw);
    player->set_gait(get_gait_from_id(prsp.gait()));

    name = player->get_name();

    ChunkPos c_pos = get_chunk_pos(x, z);
    // update other player pos;
    std::vector<std::shared_ptr<Session>> other;
    auto players = m_players_manager.snapshot();
    if (!players) {
        Logger::error("Can't get players map");
        return;
    }
    for (auto& [o_uuid, player] : *players) {
        if (o_uuid == uuid) {
            continue;
        }
        if (player->has_player(c_pos)) {
            other.emplace_back(player->get_session());
        }
    }

    Arena arena;
    auto* rsp = Arena::Create<PlayerInfoRsp>(&arena);
    rsp->set_uuid(uuid);
    rsp->set_name(name);
    auto* pos = rsp->mutable_pos();
    pos->set_x(x);
    pos->set_y(y);
    pos->set_z(z);
    rsp->set_yaw(yaw);
    rsp->set_pitch(pitch);
    rsp->set_gait(prsp.gait());
    auto packet = make_packet(*rsp);

    for (auto& session : other) {
        if (!session) {
            continue;
        }

        session->send(packet, 0);
    }
}

void ServerWorld::sync_player_water_sound(const PlayerWaterSound& rsp) {
    auto x = rsp.pos().x();
    auto y = rsp.pos().y();
    auto z = rsp.pos().z();
    ChunkPos pos = get_chunk_pos(x, z);
    auto uuid = rsp.uuid();
    auto underwater = rsp.underwater();

    std::vector<std::shared_ptr<Session>> other;
    auto players = m_players_manager.snapshot();

    if (!players) {
        Logger::error("Can't get players map");
        return;
    }

    for (auto& [o_uuid, player] : *players) {
        if (o_uuid == uuid) {
            continue;
        }
        if (player->has_player(pos)) {
            other.emplace_back(player->get_session());
        }
    }

    Arena arena;
    auto* r = Arena::Create<PlayerWaterSound>(&arena);
    r->set_uuid(uuid);
    r->set_underwater(underwater);
    auto* p = r->mutable_pos();

    p->set_x(x);
    p->set_y(y);
    p->set_z(z);
    auto packet = make_packet(*r);
    for (auto& session : other) {
        if (!session) {
            continue;
        }

        session->send(packet, 5);
    }
}

void ServerWorld::handle_player_login(const std::string& name,
                                      std::shared_ptr<Session> session) {
    std::string uuid = generate_uuid();
    Logger::info("Player {} (uuid {}) join the world", name, uuid);
    bool sucess = true;

    auto player = std::make_shared<ServerPlayer>(name, uuid, *this, session,
                                                 m_game_ticks);
    bool inserted = m_players_manager.add(std::move(player));
    if (!inserted) {
        Logger::error("Player insert Fail");
    }
    sucess = inserted;

    Arena arena;
    if (!sucess) {
        auto* rsp = Arena::Create<LoginRsp>(&arena);
        rsp->set_success(false);
        session->send(make_packet(*rsp), 0);
        return;
    }
    // Pre-insert into new_chunks to ensure correct addition to waiting_player
    /*ChunkPosSet required_chunks;
    compute_required_chunks(required_chunks, uuid);
    std::vector<ChunkPos> need_gen_chunks_pos;

    sync_and_collect_missing_chunks(need_gen_chunks_pos, required_chunks);

    {
        std::lock_guard lock(m_new_chunk_mutex);
        for (auto& pos : need_gen_chunks_pos) {
            m_new_chunks.emplace(pos, ServerChunk(*this, pos));
        }
    }
    */
    request_generation(uuid);

    auto* rsp = Arena::Create<LoginRsp>(&arena);
    rsp->set_success(true);
    rsp->set_uuid(uuid);
    rsp->set_voice_chat(m_voice_chat);
    session->send(make_packet(*rsp), 0);

    boardcast_message("Server", std::format("Player {} Join Game", name),
                      Color::YELLOW, true);
    m_entity_manager.handle_player_login(session);
}

void ServerWorld::handle_player_exit(const std::string& uuid) {
    auto pool = m_net_thread_pool.load();
    if (!pool) {
        Logger::error("Net Pool Can't find");
        player_exit(uuid);
        return;
    }
    pool->enqueue([this, uuid]() { player_exit(uuid); });
}

glm::vec3 ServerWorld::get_player_pos(const std::string& uuid) const {
    auto pos = m_players_manager.get_position(uuid);

    if (!pos) {
        Logger::error("Can't find player uuid {}", uuid);
        return glm::vec3{0.0f};
    }

    return *pos;
}

void ServerWorld::handle_chunk_req(int task_id, const std::string& uuid,
                                   ChunkPos pos) {

    auto player = m_players_manager.find(uuid);
    if (!player) {
        return;
    }

    player->update_task_id_max(task_id);

    auto pool = m_net_thread_pool.load();
    pool->enqueue([task_id, uuid, pos, this]() {
        m_chunk_system.send_chunk(task_id, uuid, pos);
    });
}

void ServerWorld::handle_chat_message(ChatMsg& msg) {

    std::string name = msg.name();
    std::string message = msg.msg();
    auto pool = m_net_thread_pool.load();
    pool->enqueue([this, player = std::move(name), m = std::move(message)]() {
        boardcast_message(player, m);
    });
}

void ServerWorld::handle_voice_message(VoiceMsg& msg) {
    ZoneScopedN("ServerWorld::handle_voice_message");
    if (!m_voice_chat) {
        return;
    }
    auto pool = m_net_thread_pool.load();
    std::string uuid = msg.uuid();
    std::string data = msg.opus_data();
    auto pos = msg.pos();
    glm::vec3 p{pos.x(), pos.y(), pos.z()};
    pool->enqueue([this, uuid = std::move(uuid), data = std::move(data), p]() {
        std::vector<std::shared_ptr<Session>> session;
        auto players = m_players_manager.snapshot();
        if (!players) {
            Logger::error("Can't get players map");
            return;
        }
        for (auto& [key, player] : *players) {
            if (key == uuid) {
                continue;
            }
            if (Math::distance2(p, player->get_pos()) > 48.0f * 48.0f) {
                continue;
            }
            session.emplace_back(player->get_session());
        }

        Arena arena;
        auto msg = Arena::Create<VoiceMsg>(&arena);
        msg->set_uuid(uuid);

        msg->set_opus_data(data);

        auto pos = msg->mutable_pos();
        pos->set_x(p.x);
        pos->set_y(p.y);
        pos->set_z(p.z);
        auto packet = make_packet(*msg);
        for (auto& s : session) {
            s->send(packet, 5);
        }
    });
}

void ServerWorld::handle_block_change(const BlockChangeReq& req) {
    ZoneScopedN("ServerWorld::handle_block_change");
    float x = std::floor(req.pos().x());
    float y = std::floor(req.pos().y());
    float z = std::floor(req.pos().z());
    if (!set_block(glm::ivec3(x, y, z), req.block())) {
        return;
    }

    Arena arena;
    BlockChangeRsp* rsp = Arena::Create<BlockChangeRsp>(&arena);
    auto* pos = rsp->mutable_pos();
    pos->set_x(x);
    pos->set_y(y);
    pos->set_z(z);
    rsp->set_block(req.block());
    std::vector<std::shared_ptr<Session>> sessions;
    auto chunk_pos = get_chunk_pos(x, z);

    auto players = m_players_manager.snapshot();

    if (!players) {
        Logger::error("Can't get players map");
        return;
    }

    for (auto& [uuid, player] : *players) {
        if (player->has_player(chunk_pos)) {
            auto session = player->get_session();
            sessions.emplace_back(std::move(session));
        }
    }

    auto packet = make_packet(*rsp);
    for (auto& x : sessions) {
        if (x) {
            x->send(packet, 1);
        }
    }
}

void ServerWorld::handle_entity_create(C2SEntityCreateRequest& req) {

    m_entity_manager.add_creature(req.name(), Tools::get_net_vec3(req.pos()));
}
void ServerWorld::handle_entity_destory(C2SEntityDestoryRequest& req) {
    m_entity_manager.destory(req.id());
}

int ServerWorld::rendering_distance() const {
    return m_chunk_system.render_distance();
}

void ServerWorld::rendering_distance(int rendering_distance) {
    m_chunk_system.set_render_distance(rendering_distance);
}

CaveCarver& ServerWorld::cave_carcer() { return m_cave_carcer; }
RiverWorm& ServerWorld::river_worm() { return m_river_worm; }

Config& ServerWorld::get_config() { return m_config; }

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
int ServerWorld::gen_pool_threads() const {
    return m_chunk_system.generation_threads();
}
int ServerWorld::max_threads() const { return m_max_threads.load(); }

void ServerWorld::change_pool_threads(ThreadPoolKind, int) {
    Logger::error("ServerWorld::change_pool_threads is Deprecated");
}

int ServerWorld::change_pool_threads(
    std::atomic<std::shared_ptr<ThreadPool>>& thread_pool, int threads) {
    m_max_threads = std::thread::hardware_concurrency();
    if (m_max_threads < 1) {
        Logger::warn("Can't Get Max Support Threads, Set Max Threads to 4");
        m_max_threads = 1;
    }
    int used_thread = std::clamp(threads, 1, m_max_threads.load());
    Logger::info("Create New Thread Pool Use {} Threads", used_thread);
    thread_pool.store(std::make_shared<ThreadPool>(used_thread));
    return used_thread;
}

int ServerWorld::change_pool_threads(
    std::atomic<std::shared_ptr<PriorityThreadPool>>& thread_pool,
    int threads) {
    m_max_threads = std::thread::hardware_concurrency();
    if (m_max_threads < 1) {
        Logger::warn("Can't Get Max Support Threads, Set Max Threads to 4");
        m_max_threads = 1;
    }
    int used_thread = std::clamp(threads, 1, m_max_threads.load());
    thread_pool.store(std::make_shared<PriorityThreadPool>(used_thread));
    return used_thread;
}

void ServerWorld::send_server_stop() {
    Arena arena;
    auto* rsp = Arena::Create<LogoutRsp>(&arena);
    rsp->set_server_stop(true);
    auto sessions = get_all_session();
    auto packet = make_packet(*rsp);
    for (auto& s : sessions) {
        s->send(packet, 0);
    }
    Logger::info("Send Server Mesaage Success");
}

void ServerWorld::boardcast_message(const std::string& name,
                                    const std::string& message, Color color,
                                    bool system_msg) {

    std::vector<std::shared_ptr<Session>> session =
        m_players_manager.get_all_session();

    Arena arena;
    auto msg = Arena::Create<ChatMsg>(&arena);
    if (m_enable_filter) {
        msg->set_msg(m_filter.filter(message));
        msg->set_name(m_filter.filter(name));
    } else {
        msg->set_msg(message);
        msg->set_name(name);
    }

    msg->set_color(std::to_underlying(color));
    msg->set_system_msg(system_msg);
    auto packet = make_packet(*msg);
    for (auto& s : session) {
        s->send(packet);
    }
}

void ServerWorld::player_exit(const std::string& uuid) {
    auto player = m_players_manager.remove(uuid);

    if (!player) {
        return; // Already removed
    }

    const std::string NAME = player->get_name();
    auto exit_session = player->get_session();

    Logger::info("Player {} Exit the Server", NAME);

    m_chunk_system.release_chunk(player);

    Arena arena;
    auto* rsp = Arena::Create<LogoutRsp>(&arena);
    rsp->set_uuid(uuid);
    rsp->set_server_stop(false);
    exit_session->send(make_packet(*rsp), 0);

    auto sessions = get_all_session();

    auto packet = make_packet(*rsp);
    for (auto& s : sessions) {
        if (s) {
            s->send(packet, 0);
        }
    }

    boardcast_message("Server", std::format("Player {} Exit Game", NAME),
                      Color::YELLOW, true);
}

int ServerWorld::chunk_load_style() const {
    return std::to_underlying(m_chunk_system.load_style());
}
void ServerWorld::set_chunk_load_style(int id) {
    m_chunk_system.set_load_style(id);
}

int ServerWorld::chunk_size() const { return m_chunk_system.chunk_size(); }

tbb::concurrent_vector<std::shared_ptr<Session>>
ServerWorld::get_all_session() const {
    tbb::concurrent_vector<std::shared_ptr<Session>> sessions;
    auto players = m_players_manager.snapshot();
    if (!players) {
        Logger::error("Can't get players map");
        return {};
    }
    for (const auto& [_, player] : *players) {
        sessions.emplace_back(player->get_session());
    }
    return sessions;
}

uint32_t ServerWorld::get_chunk_ref_count(const glm::vec3& pos) const {
    ChunkPos p = get_chunk_pos(pos.x, pos.z);

    return m_chunk_system.get_chunk_ref_count(p);
}

int ServerWorld::get_block(const glm::ivec3& block_pos) const {
    return m_chunk_system.get_block(block_pos);
}
bool ServerWorld::is_solid(const glm::ivec3& block_pos) const {
    return m_chunk_system.is_solid(block_pos);
}
bool ServerWorld::can_pass_block(const glm::ivec3& block_pos) const {
    return m_chunk_system.can_pass_block(block_pos);
}

BlockType ServerWorld::get_block_tpye(const glm::ivec3& block_pos) const {
    return m_chunk_system.get_block_type(block_pos);
}

int ServerWorld::get_per_tick_time() const { return m_per_tick_time; }
ServerEntityManager& ServerWorld::entity_manager() { return m_entity_manager; }
ServerPlayerManager& ServerWorld::player_manager() { return m_players_manager; }
ServerChunkSystem& ServerWorld::chunk_system() { return m_chunk_system; }
std::shared_ptr<ThreadPool> ServerWorld::get_compute_pool() {
    return m_compute_thread_pool.load();
}

size_t ServerWorld::player_sum() const { return m_players_manager.sum(); }
RunMode ServerWorld::get_runmode() const { return m_runmode; }
} // namespace Cubed