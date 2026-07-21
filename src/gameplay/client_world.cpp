#include "Cubed/gameplay/client_world.hpp"

#include "Cubed/config.hpp"
#include "Cubed/gameplay/chunk_generator.hpp"
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/packet.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/tools/math_tools.hpp"
#include "Cubed/tools/time_tools.hpp"

#include <absl/container/inlined_vector.h>
#include <numbers>

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace google::protobuf;
namespace Cubed {

namespace {
struct ChunkRenderData {
    std::array<const std::vector<BlockType>*, 4> neighbor_block;
    ClientChunk* chunk;
};
} // namespace

ClientWorld::ClientWorld(AudioEngine& auido, Config& config, WorldScene& scene)
    : m_player(*this), m_audio(auido), m_config(config), m_world_scene(scene) {}

ClientWorld::~ClientWorld() {
    m_client->close();

    stop_client_thread();
    stop_thread_pool();
    // Must first clean up and push the generated chunk data into
    // m_pending_delete_vbo and m_pending_delete_vao; cannot delete them in the
    // destructor, otherwise it will cause leaks and use-after-free.
    m_dirty_chunk_queue.clear();
    m_pending_upload_queue.clear();

    m_chunks.clear();

    if (m_is_pending_delete_queue_free.exchange(true)) {
        return;
    }

    {
        std::lock_guard lk(m_delete_vbo_mutex);
        m_pending_delete_vbo.clear();
    }
    {
        std::lock_guard lk(m_delete_vao_mutex);
        m_pending_delete_vao.clear();
    }
    m_ticktimers.clear();
}

const std::optional<LookBlock>& ClientWorld::get_look_block_pos() const {

    return m_player.get_look_block_pos();
}

ClientPlayer& ClientWorld::get_player() { return m_player; }
const ClientPlayer& ClientWorld::get_player() const { return m_player; }
int ClientWorld::get_block(const glm::ivec3& block_pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(block_pos.x, block_pos.z);
    chunk_cacc cacc;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        return 0;
    }

    const auto& chunk_blocks = cacc->second->get_chunk_blocks();
    auto [x, y, z] = ClientChunk::world_to_block(block_pos, {chunk_x, chunk_z});
    if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
        z >= CHUNK_SIZE) {
        return 0;
    }
    return chunk_blocks[ClientChunk::index(x, y, z)];
}
bool ClientWorld::is_solid(const glm::ivec3& block_pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(block_pos.x, block_pos.z);
    chunk_cacc cacc;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        return false;
    }
    const auto& chunk_blocks = cacc->second->get_chunk_blocks();
    auto [x, y, z] = ClientChunk::world_to_block(block_pos, {chunk_x, chunk_z});
    if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
        z >= CHUNK_SIZE) {
        return false;
    }
    auto id = chunk_blocks[ClientChunk::index(x, y, z)];
    if (BlockManager::is_gas(id) || BlockManager::is_liquid(id)) {
        return false;
    } else {
        return true;
    }
}
bool ClientWorld::can_pass_block(const glm::ivec3& block_pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(block_pos.x, block_pos.z);
    chunk_cacc cacc;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        return true;
    }
    const auto& chunk_blocks = cacc->second->get_chunk_blocks();
    auto [x, y, z] = ClientChunk::world_to_block(block_pos, {chunk_x, chunk_z});
    if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
        z >= CHUNK_SIZE) {
        return true;
    }
    auto id = chunk_blocks[ClientChunk::index(x, y, z)];
    return BlockManager::is_passable(id);
}

void ClientWorld::rebuild_world() {
    if (m_is_rebuilding.exchange(true)) {
        return;
    }

    stop_client_thread();
    stop_thread_pool();

    m_chunks.clear();

    m_pending_upload_queue.clear();

    start_thread_pool();
    start_client_thread(m_player.get_uuid());
    request_chunk();
    m_is_rebuilding = false;
}

BlockType ClientWorld::get_block_tpye(const glm::ivec3& block_pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(block_pos.x, block_pos.z);
    chunk_cacc cacc;
    ;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        // Logger::error("Can't Find Block {} {} {}", block_pos.x, block_pos.y,
        //               block_pos.z);
        return 0;
    }
    const auto& chunk_blocks = cacc->second->get_chunk_blocks();
    auto [x, y, z] = ClientChunk::world_to_block(block_pos, {chunk_x, chunk_z});
    if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
        z >= CHUNK_SIZE) {
        // Logger::error("Can't Find Block {} {} {}", block_pos.x, block_pos.y,
        //               block_pos.z);
        return 0;
    }
    return chunk_blocks[ClientChunk::index(x, y, z)];
}
void ClientWorld::set_block(const glm::ivec3& block_pos, unsigned id) {
    int world_x, world_y, world_z;
    world_x = block_pos.x;
    world_y = block_pos.y;
    world_z = block_pos.z;

    auto [chunk_x, chunk_z] = get_chunk_pos(world_x, world_z);
    ChunkPos pos{chunk_x, chunk_z};

    BlockType origin_id = 0;
    {
        chunk_acc acc;

        if (!m_chunks.find(acc, pos)) {
            return;
        }
        auto [x, y, z] = ClientChunk::world_to_block(world_x, world_y, world_z,
                                                     chunk_x, chunk_z);
        if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
            z >= CHUNK_SIZE) {
            return;
        }
        int idx = ClientChunk::index(x, y, z);
        origin_id = acc->second->get_chunk_block(idx);
        acc->second->set_chunk_block(idx, id);
        acc->second->mark_dirty();
    }

    glm::vec3 sound_pos{world_x + 0.5f, world_y + 0.5f, world_z + 0.5f};

    if (id == 0) {
        std::string name = BlockManager::name_form_id(origin_id);
        std::string sound = "block/" + name + "/break.ogg";
        m_pending_sound.emplace(sound, sound_pos);
    } else {
        std::string name = BlockManager::name_form_id(id);
        std::string sound = "block/" + name + "/place.ogg";
        m_pending_sound.emplace(sound, sound_pos);
    }

    auto pool = m_thread_pool.load();

    pool->enqueue(0, [this, pos]() {
        std::shared_ptr<ClientChunk> chunk;

        {
            chunk_acc acc;
            if (m_chunks.find(acc, pos)) {
                chunk = acc->second;
            }
        }

        if (!chunk) {
            return;
        }

        OptionalBlockVectorArray neighbor_block;
        for (int i = 0; i < 4; i++) {
            chunk_cacc cacc;
            if (m_chunks.find(cacc, pos + CHUNK_DIR[i])) {
                neighbor_block[i] = (cacc->second->get_chunk_blocks());
            } else {
                neighbor_block[i] = std::nullopt;
            }
        }

        chunk->gen_vertex_data(neighbor_block);
        m_dirty_chunk_queue.emplace(pos);
    });

    static const glm::ivec3 NEIGHBOR_DIRS[] = {
        {1, 0, 0}, {-1, 0, 0}, {0, 0, -1}, {0, 0, 1}};
    static constexpr int NPOS_SUM = sizeof(NEIGHBOR_DIRS);

    absl::InlinedVector<ChunkPos, NPOS_SUM> nposes;

    for (const auto& dir : NEIGHBOR_DIRS) {
        glm::ivec3 neighbor = block_pos + dir;

        auto [cx, cz] = get_chunk_pos(neighbor.x, neighbor.z);
        {
            chunk_acc acc;
            if (m_chunks.find(acc, {cx, cz})) {
                if (acc->second->is_dirty()) {
                    continue;
                }
                nposes.emplace_back(acc->first);
            }
        }
    }

    for (auto& npos : nposes) {
        pool->enqueue(0, [this, npos]() {
            std::shared_ptr<ClientChunk> chunk;

            {
                chunk_acc acc;
                if (m_chunks.find(acc, npos)) {
                    chunk = acc->second;
                }
            }

            if (!chunk) {
                return;
            }

            OptionalBlockVectorArray neighbor_block;
            for (int i = 0; i < 4; i++) {
                chunk_cacc cacc;
                if (m_chunks.find(cacc, npos + CHUNK_DIR[i])) {
                    neighbor_block[i] = (cacc->second->get_chunk_blocks());
                } else {
                    neighbor_block[i] = std::nullopt;
                }
            }

            chunk->gen_vertex_data(neighbor_block);

            m_dirty_chunk_queue.emplace(npos);
        });
    }
}
void ClientWorld::push_delete_vbo(std::unique_ptr<VertexBuffer>& vbo) {
    if (m_is_pending_delete_queue_free) {
        Logger::error("Push delete vbo Use After Free");
        return;
    }
    std::lock_guard lk(m_delete_vbo_mutex);
    m_pending_delete_vbo.push_back(std::move(vbo));
}
void ClientWorld::push_delete_vao(std::unique_ptr<VertexArray>& vao) {
    if (m_is_pending_delete_queue_free) {
        Logger::error("Push delete vao Use After Free");
        return;
    }
    std::lock_guard lk(m_delete_vao_mutex);
    m_pending_delete_vao.push_back(std::move(vao));
}

void ClientWorld::report_block_change(const glm::ivec3& pos,
                                      unsigned id) const {
    if (id != 0) {
        AABB block_box = get_block_aabb(pos);
        std::shared_lock lock(m_player_info_mutex);

        for (auto& [uuid, player] : m_player_info) {
            AABB box = ClientPlayer::get_aabb(player.target_pos);
            if (box.intersects(block_box)) {
                return;
            }
        }
    }

    Arena arena;
    auto* req = Arena::Create<BlockChangeReq>(&arena);
    req->set_uuid(m_player.get_uuid());
    req->set_block(id);
    auto* p = req->mutable_pos();
    p->set_x(pos.x);
    p->set_y(pos.y);
    p->set_z(pos.z);
    m_client->send(make_packet(*req), 0);
}

void ClientWorld::receive_block_change(const BlockChangeRsp& rsp) {
    glm::vec3 pos{rsp.pos().x(), rsp.pos().y(), rsp.pos().z()};
    set_block(pos, rsp.block());
}

void ClientWorld::receive_time(const UpdateTime& rsp) {
    m_game_ticks = rsp.game_tick();
    m_day_tick = rsp.day_tick();
}

void ClientWorld::receive_remote_player(const PlayerInfoRsp& rsp) {
    auto pitch = rsp.pitch();
    auto yaw = rsp.yaw();
    {
        std::lock_guard lock(m_player_info_mutex);
        glm::vec3 pos{rsp.pos().x(), rsp.pos().y(), rsp.pos().z()};
        auto it = m_player_info.find(rsp.uuid());
        if (it == m_player_info.end()) {
            m_player_info.emplace(
                std::piecewise_construct, std::forward_as_tuple(rsp.uuid()),
                std::forward_as_tuple(rsp.name(), rsp.uuid(), pos, pos, yaw,
                                      yaw, pitch, pitch,
                                      get_gait_from_id(rsp.gait())));
        } else {
            it->second.target_pos = pos;
            it->second.yaw = yaw;
            it->second.pitch = pitch;
            it->second.gait = get_gait_from_id(rsp.gait());
        }
        // Logger::info("Player {} pos Update", rsp.name());
    }
}

void ClientWorld::receive_player_logout(const LogoutRsp& rsp) {
    if (rsp.server_stop()) {
        m_receive_exit = true;
        return;
    }
    if (rsp.uuid() == m_player.get_uuid()) {
        m_receive_exit = true;
        return;
    }
    {
        std::lock_guard lock(m_player_info_mutex);
        int sum = m_player_info.erase(rsp.uuid());
        if (sum == 0) {
            Logger::warn("Player {} not find", rsp.uuid());
        } else {
            Logger::info("Player {} erase", rsp.uuid());
        }
    }
}

void ClientWorld::receive_player_water_sound(const PlayerWaterSound& rsp) {
    if (rsp.uuid() == m_player.get_uuid()) {
        return;
    }
    glm::vec3 pos = {rsp.pos().x(), rsp.pos().y(), rsp.pos().z()};

    Logger::info("Client: Receive Player Water Sound");

    m_pending_sound.emplace("ambient/water/in_and_out_of_water.flac", pos);
}

void ClientWorld::send_player_water_sound(bool underwater,
                                          const glm::vec3& pos) {
    Arena arena;
    auto* r = Arena::Create<PlayerWaterSound>(&arena);

    r->set_underwater(underwater);
    auto* p = r->mutable_pos();
    p->set_x(pos.x);
    p->set_y(pos.y);
    p->set_z(pos.z);
    r->set_uuid(m_player.get_uuid());

    m_client->send(make_packet(*r));
    Logger::info("Client: Send Player Water Sound");
}

void ClientWorld::init(std::string_view player_name,
                       std::shared_ptr<NetworkClient> client) {
    m_player.init(player_name);
    m_client = client;

    reload_config(false);

    m_random.init(ChunkGenerator::seed());

    // timer
    register_ticktimer("player_pos", 1, [this]() { report_player_info(); });
    m_timers.try_emplace("Birds Sound", 60.0f, [this]() {
        auto player_pos = m_player.get_player_pos();
        if (player_pos.y < SEA_LEVEL) {
            return;
        }
        ChunkPos pos = get_chunk_pos(player_pos.x, player_pos.z);
        {
            chunk_cacc cacc;
            if (m_chunks.find(cacc, pos)) {
                if (cacc->second->get_biome() == BiomeType::FOREST) {
                    m_audio.play_2d("ambient/birds.ogg", true);
                }
            }
        }
    });

    m_timers.try_emplace("Ocean Wave", 3.0f, [this]() {
        auto player_pos = m_player.get_player_pos();
        if (player_pos.y < SEA_LEVEL - 10 || player_pos.y > SEA_LEVEL + 10) {
            return;
        }

        auto ans = m_random.random_int(1, 4);
        std::string sound =
            "ambient/ocean/wave00" + std::to_string(ans) + ".flac";
        ChunkPos pos = get_chunk_pos(player_pos.x, player_pos.z);
        {
            chunk_cacc cacc;
            if (m_chunks.find(cacc, pos)) {
                if (cacc->second->get_biome() == BiomeType::OCEAN) {
                    m_audio.play_2d(sound, true);
                }
            }
        }
    });

    m_timers.try_emplace("under water bubble", 1.5f, [this]() {
        if (m_player.is_underwater()) {
            auto ans = m_random.random_int(1, 2);
            std::string sound =
                "ambient/water/bubble00" + std::to_string(ans) + ".ogg";
            m_audio.play_3d(sound, m_player.get_player_pos(), true);
        }
    });

    m_timers.try_emplace("bgm change", 350.0f, [this]() {
        if (m_day_tick >= 17000 || m_day_tick < 5000) {
            m_audio.change_bgm("bgm/bgm002.ogg");
        } else {
            m_audio.change_bgm("bgm/bgm001.mp3");
        }
    });

    LoginReq req;
    req.set_name(m_player.get_name());
    while (!client->is_connected()) {
        if (client->is_connect_error()) {
            throw std::runtime_error(client->get_error_string());
        }
        std::this_thread::sleep_for(milliseconds(200));
    }
    start_thread_pool();
    // request login
    Logger::info("Send Login Request");
    m_client->send(make_packet(req), 0);
    m_audio.play_bgm();
}

void ClientWorld::receive_login_rsp(LoginRsp& rsp) {
    m_voice_chat = rsp.voice_chat();
    start_client_thread(rsp.uuid());
}

void ClientWorld::start_client_thread(std::string_view uuid) {
    if (m_game_running) {
        Logger::error("Game Already Running");
        return;
    }
    // response
    m_player.set_uuid(uuid);

    m_client_thread = std::jthread([this](std::stop_token token) {
        m_game_running = true;
        client_run(token);
    });

    // Wait for 20 ticks, after the server's central chunk is generated, then
    // request chunks

    std::this_thread::sleep_for(milliseconds(20 * DEFAULT_PER_TICK_TIME));

    request_chunk();
}

void ClientWorld::stop_client_thread() {
    m_client_thread.request_stop();
    if (m_client_thread.joinable()) {
        m_client_thread.join();
    }
    m_game_running = false;
}
void ClientWorld::start_thread_pool() {
    int max_threads = std::thread::hardware_concurrency();
    int threads = std::min<size_t>(max_threads, 4);
    change_pool_threads(threads);
}
void ClientWorld::stop_thread_pool() {
    auto pool_ptr = m_thread_pool.load();
    if (pool_ptr) {
        pool_ptr->stop();
    }
    m_thread_pool.store(nullptr);
    Logger::info("Thread Pool Stopped");
}

void ClientWorld::change_pool_threads(int threads) {
    int m_max_threads = std::thread::hardware_concurrency();
    if (m_max_threads < 1) {
        Logger::warn("Can't Get Max Support Threads, Set Max Threads to 4");
        m_max_threads = 1;
    }
    int used_thread = std::clamp(threads, 1, m_max_threads);
    Logger::info("Create New Thread Pool Use {} Threads", used_thread);
    m_thread_pool.store(std::make_shared<PriorityThreadPool>(used_thread));
}

void ClientWorld::reload_config(bool chunk_build) {
    int dist = m_config.get<int>("world.rendering_distance", PRE_LOAD_DISTANCE);
    Logger::info("Get Config Randering dist {}", dist);
    m_rendering_distance = dist <= MAX_DISTANCE ? dist : MAX_DISTANCE;
    if (chunk_build) {
        request_chunk();
    }

    m_player.reload_config();
}

void ClientWorld::client_run(std::stop_token stoken) {
    Logger::info("Client Thread Started");
    using Clock = std::chrono::steady_clock;

    constexpr auto TICK = std::chrono::milliseconds(DEFAULT_PER_TICK_TIME);

    auto next = Clock::now();
    while (!stoken.stop_requested()) {
        next += TICK;
        for (auto& x : m_ticktimers) {
            x.second.update();
        }
        std::this_thread::sleep_until(next);
    }
}

void ClientWorld::report_player_info() {
    if (!m_client) {
        return;
    }
    Arena arena;
    auto* info = Arena::Create<C2S_PlayerInfo>(&arena);
    info->set_uuid(m_player.get_uuid());
    glm::vec3 player_pos = m_player.get_player_pos();
    auto* v3 = info->mutable_pos();
    v3->set_x(player_pos.x);
    v3->set_y(player_pos.y);
    v3->set_z(player_pos.z);
    info->set_yaw(m_player.yaw());
    info->set_pitch(m_player.pitch());
    info->set_gait(get_gait_id(m_player.get_gait()));

    m_client->send(make_packet(*info), 0);
}

void ClientWorld::update_chunk(const ChunkPosSet& old, const ChunkPosSet& now) {

    // Elements in the old set that are not contained in now are not needed by
    // the current player.

    for (auto& pos : old) {
        if (!now.contains(pos)) {

            chunk_acc acc;
            if (!m_chunks.find(acc, pos)) {
                Logger::warn("Update Ref Count Error, can't Find old pos "
                             "in m_chunks");
                continue;
            }

            m_chunks.erase(acc);
        }
    }
}

void ClientWorld::request_chunk() {
    if (m_requesting_chunk.exchange(true)) {
        Logger::warn("It is requesting new chunk!");
        return;
    }
    ChunkPosSet required_chunks;

    glm::vec3 player_pos = m_player.get_player_pos();

    int x = std::floor(player_pos.x);
    int z = std::floor(player_pos.z);
    auto [chunk_x, chunk_z] = get_chunk_pos(x, z);
    int radius = m_rendering_distance;
    Logger::info("Client Chunk Radius {}", radius);
    int r2 = radius * radius;
    required_chunks.reserve(radius * radius);

    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dz = -radius; dz <= radius; ++dz) {
            if (dx * dx + dz * dz <= r2) {
                required_chunks.emplace(chunk_x + dx, chunk_z + dz);
            }
        }
    }

    ChunkPosSet old = m_player.get_chunk_pos_set();
    m_player.update_chunk_set(required_chunks);

    ChunkPosVector need_send_pos;

    for (auto pos : required_chunks) {
        chunk_cacc cacc;
        if (!m_chunks.find(cacc, pos)) {
            need_send_pos.emplace_back(pos);
        }
    }

    update_chunk(old, required_chunks);

    if (need_send_pos.empty()) {
        m_requesting_chunk = false;
        return;
    }
    using enum ChunkLoadStyle;
    switch (m_chunk_load_style) {
    case RANDOM:

        break;
    case CENTER: {

        glm::vec3 player_pos = m_player.get_player_pos();
        ChunkPos player_chunk_pos = get_chunk_pos(player_pos.x, player_pos.z);
        auto dist2 = [player_chunk_pos](ChunkPos chunk_pos) {
            float dx = player_chunk_pos.x - chunk_pos.x;
            float dz = player_chunk_pos.z - chunk_pos.z;
            return dx * dx + dz * dz;
        };

        std::sort(need_send_pos.begin(), need_send_pos.end(),
                  [&dist2](const auto& a, const auto& b) {
                      return dist2(a) < dist2(b);
                  });
    }
    }
    auto uuid = m_player.get_uuid();
    Arena arena;
    ++m_chunk_task_id;
    auto* req = Arena::Create<ChunkDataReq>(&arena);
    for (const auto& pos : need_send_pos) {
        req->set_task_id(m_chunk_task_id.load());
        req->set_uuid(uuid);
        auto* p = req->mutable_pos();
        p->set_x(pos.x);
        p->set_z(pos.z);
        m_client->send(make_packet(*req));
    }
    Logger::info("Send Chunk Request Success");
    m_requesting_chunk = false;
}
void ClientWorld::reset_key_status() { m_player.reset_key_status(); }
void ClientWorld::receive_chunk(std::vector<uint8_t> raw_data,
                                PacketHeader header) {

    // vertex data will genrator in  client thread pool instead of net thread;
    auto pool = m_thread_pool.load();
    if (!pool) {
        Logger::error("Client Thread Pool is nullptr");
        return;
    }
    pool->enqueue(
        [this, raw_data = std::move(raw_data), header = std::move(header)]() {
            Arena arena;
            auto* data = Arena::Create<ChunkDataRsp>(&arena);
            if (!decode_packet(*data, raw_data, header)) {
                return;
            }

            if (data->task_id() < m_chunk_task_id) {
                return;
            }

            {
                chunk_cacc cacc;
                ChunkPos pos{data->pos().x(), data->pos().z()};
                if (m_chunks.find(cacc, pos)) {
                    Logger::warn("Chunk {} {} has already in client world",
                                 pos.x, pos.z);
                    return;
                }
            }

            std::unique_ptr<ClientChunk> chunk =
                std::make_unique<ClientChunk>(*this);
            chunk->receive_chunk(*data);

            m_pending_upload_queue.emplace(std::move(chunk));
        });
}
bool ClientWorld::is_receive_exit() { return m_receive_exit; }

int ClientWorld::chunk_size() const { return m_chunks.size(); }

AABB ClientWorld::get_block_aabb(const glm::ivec3& pos) {
    auto x = pos.x;
    auto y = pos.y;
    auto z = pos.z;
    return {glm::vec3{static_cast<float>(x), static_cast<float>(y),
                      static_cast<float>(z)},
            glm::vec3{static_cast<float>(x + 1), static_cast<float>(y + 1),
                      static_cast<float>(z + 1)}};
}

AudioEngine& ClientWorld::get_audio() { return m_audio; }
const AudioEngine& ClientWorld::get_audio() const { return m_audio; }
Config& ClientWorld::get_config() { return m_config; }
WorldScene& ClientWorld::world_scene() { return m_world_scene; }
void ClientWorld::set_direct_exit() { m_exit_direct = true; }
void ClientWorld::request_exit() {
    if (m_receive_exit) {
        return;
    }
    Arena arena;
    auto* req = Arena::Create<LogoutReq>(&arena);
    req->set_uuid(m_player.get_uuid());
    m_client->send(make_packet(*req));
    int cnt = 0;
    while (!m_receive_exit) {
        if (m_client->is_connect_error() || m_exit_direct) {
            break;
        }
        std::this_thread::sleep_for(milliseconds(DEFAULT_PER_TICK_TIME));
        ++cnt;
        if (cnt >= WORLD_EXIT_TIMEOUT) {
            Logger::warn("Can't Receive Server Exit Sign");
            break;
        }
    }
}

void ClientWorld::receive_chat_message(ChatMsg& msg) {
    Color color = color_from_int(msg.color());

    m_message_queue.emplace(msg.name(), msg.msg(), color, msg.system_msg(),
                            Tools::get_time_ticks());
}

void ClientWorld::receive_voice_message(VoiceMsg& msg) {
    if (!m_voice_chat) {
        return;
    }
    glm::vec3 pos{msg.pos().x(), msg.pos().y(), msg.pos().z()};
    Logger::info("Receive Voice From net");
    m_voice_queue.emplace(msg.opus_data(), pos);
}
bool ClientWorld::enable_voice_chat() const { return m_voice_chat.load(); }
void ClientWorld::send_chat_message(ChatMessage& message) {
    Arena arena;
    auto msg = Arena::Create<ChatMsg>(&arena);
    msg->set_name(message.player);
    msg->set_msg(message.text);
    m_client->send(make_packet(*msg));
}

void ClientWorld::update(float delta_time) {

    m_player.update(delta_time);
    {
        std::lock_guard lk(m_delete_vbo_mutex);
        m_pending_delete_vbo.clear();
    }

    {
        std::lock_guard lk(m_delete_vao_mutex);
        m_pending_delete_vao.clear();
    }

    std::vector<std::unique_ptr<ClientChunk>> new_chunks;
    {
        std::unique_ptr<ClientChunk> chunk;
        int sum = 0;
        while (m_pending_upload_queue.try_pop(chunk)) {
            new_chunks.emplace_back(std::move(chunk));
            ++sum;
            if (sum >= MAX_UPLOAD_CHUNK_SUM) {
                break; // Limit the maximum number of uploads per frame to
                       // improve frame rate performance
            }
        }
    }

    for (auto& c : new_chunks) {
        c->upload_to_gpu();
    }

    for (auto& c : new_chunks) {
        m_chunks.emplace(c->get_chunk_pos(), std::move(c));
    }
    m_render_snapshots.clear();

    ChunkPos pos;

    while (m_dirty_chunk_queue.try_pop(pos)) {
        std::shared_ptr<ClientChunk> chunk;
        {
            chunk_acc acc;
            if (m_chunks.find(acc, pos)) {
                chunk = acc->second;
            }
        }
        if (!chunk) {
            continue;
        }

        chunk->upload_to_gpu();
    }

    auto chunk_pos_set = m_player.get_chunk_pos_set();

    for (auto& pos : chunk_pos_set) {
        std::shared_ptr<ClientChunk> chunk;
        {
            chunk_acc acc;
            if (m_chunks.find(acc, pos)) {
                chunk = acc->second;
            }
        }
        if (!chunk) {
            continue;
        }

        m_render_snapshots.push_back(chunk->get_render_snapshot());
    }

    m_render_player_data.clear();
    {
        std::lock_guard lock(m_player_info_mutex);
        for (auto& [uuid, player] : m_player_info) {
            player.render_pos =
                glm::mix(player.render_pos, player.target_pos, 0.15f);
            if (Math::distance2(player.render_pos, m_player.get_player_pos()) >
                m_rendering_distance * CHUNK_SIZE * m_rendering_distance *
                    CHUNK_SIZE) {
                continue;
            }
            player.render_yaw = glm::mix(player.render_yaw, player.yaw, 0.15);
            player.render_pitch =
                glm::mix(player.render_pitch, player.pitch, 0.15);

            if (player.gait == Gait::WALK || player.gait == Gait::RUN) {

                player.walk_time += delta_time;

                float speed = player.gait == Gait::RUN ? 14.0f : 8.0f;
                float amp = player.gait == Gait::RUN ? 50.0f : 35.0f;
                // float amp = 90.0f;
                player.angle =
                    glm::sin(player.walk_time * speed) * glm::radians(amp);
            } else if (player.gait == Gait::STOP) {
                float t = glm::clamp(delta_time * 10.0f, 0.0f, 1.0f);
                player.angle = glm::mix(player.angle, 0.0f, t);
            }

            // walking sound
            if (player.gait == Gait::STOP) {
                player.moving_time = 0.0f;
            } else {
                player.moving_time += delta_time;
            }
            auto play_walk_sound = [&]() {
                glm::ivec3 block = glm::floor(player.render_pos);
                block.y -= 1;
                BlockType id = get_block_tpye(block);
                if (id == 0) {
                    return;
                }
                std::string name = BlockManager::name_form_id(id);
                std::string sound = "block/" + name + "/walk.ogg";
                m_audio.play_3d(sound, player.render_pos);
            };
            if (player.gait == Gait::WALK) {
                if (player.moving_time >= ClientPlayer::WALK_SOUND_INTERVAL) {
                    player.moving_time = 0.0f;
                    play_walk_sound();
                }
            }
            if (player.gait == Gait::RUN) {
                if (player.moving_time >= ClientPlayer::RUN_SOUND_INTERVAL) {
                    player.moving_time = 0.0f;
                    play_walk_sound();
                }
            }

            m_render_player_data.emplace_back(
                player.name, player.uuid, player.render_pos, player.render_yaw,
                player.render_pitch, player.gait, player.angle);
        }
        {
            auto gait = m_player.get_gait();
            auto& walk_time = m_player.walk_time();
            auto& angle = m_player.angle();
            if (gait == Gait::WALK || gait == Gait::RUN) {

                walk_time += delta_time;

                float speed = gait == Gait::RUN ? 14.0f : 8.0f;
                float amp = gait == Gait::RUN ? 50.0f : 35.0f;
                // float amp = 90.0f;
                angle = glm::sin(walk_time * speed) * glm::radians(amp);
            } else if (gait == Gait::STOP) {
                float t = glm::clamp(delta_time * 10.0f, 0.0f, 1.0f);
                angle = glm::mix(angle, 0.0f, t);
            }

            m_render_player_data.emplace_back(
                m_player.get_name(), m_player.get_uuid(),
                m_player.get_player_pos(), m_player.yaw(), m_player.pitch(),
                m_player.get_gait(), m_player.angle());
        }
    }

    // sound
    PendingSound pending_sound;
    while (m_pending_sound.try_pop(pending_sound)) {
        m_audio.play_3d(pending_sound.sound, pending_sound.sound_pos);
    }

    for (auto& [pos, timer] : m_timers) {
        timer.update(delta_time);
    }
    ChatMessage message;
    while (m_message_queue.try_pop(message)) {
        m_world_scene.handle_chat_message(message);
    }

    VoiceMessage vm;
    while (m_voice_queue.try_pop(vm)) {
        m_audio.receive_voice(vm.data, vm.pos);
    }
}

bool ClientWorld::handle_event(const Event& e) {
    return std::visit(
        Overloaded{[this](const MouseButtonEvent& e) {
                       if (m_player.handle_mouse_button_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [](const MouseMoveEvent&) { return false; },
                   [this](const MouseWheelEvent& e) {
                       if (m_player.handle_mouse_wheel_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const KeyEvent& e) {
                       if (m_player.handle_key_event(e)) {
                           return true;
                       }

                       return false;
                   },
                   [](const TextInputEvent&) { return false; },
                   [](const WindowResizeEvent&) { return false; },
                   [](const FrameBufferResizeEvent&) { return false; }

        },
        e);
}

glm::vec3 ClientWorld::sunlight_dir() const {
    float altitude = sin((m_day_tick - 6 * PER_HOUR) /
                         static_cast<float>(DAY_TIME / 2) * std::numbers::pi) *
                     90.0f;

    float t = static_cast<float>(m_day_tick) / DAY_TIME;
    float azimuth = 90.0f - 360.0f * (t - 0.25f);

    float alt = glm::radians(altitude);
    float az = glm::radians(azimuth);
    glm::vec3 dir;
    dir.x = cos(alt) * sin(az);
    dir.y = sin(alt);
    dir.z = cos(alt) * cos(az);

    return glm::normalize(-dir);
}

bool ClientWorld::sphere_collide_world(glm::vec3 center, float radius) const {
    glm::ivec3 min = glm::floor(center - glm::vec3(radius));
    glm::ivec3 max = glm::floor(center + glm::vec3(radius));

    for (int x = min.x; x <= max.x; ++x) {
        for (int y = min.y; y <= max.y; ++y) {
            for (int z = min.z; z <= max.z; ++z) {
                if (!is_solid({x, y, z}))
                    continue;

                glm::vec3 closest;
                closest.x = glm::clamp(center.x, float(x), float(x + 1));
                closest.y = glm::clamp(center.y, float(y), float(y + 1));
                closest.z = glm::clamp(center.z, float(z), float(z + 1));

                glm::vec3 d = center - closest;

                if (glm::dot(d, d) < radius * radius)
                    return true;
            }
        }
    }

    return false;
}

int ClientWorld::rendering_distance() const {
    return m_rendering_distance.load();
}

void ClientWorld::rendering_distance(int rendering_distance) {
    m_rendering_distance = rendering_distance;
    Logger::info("Set Rendering dist {} , the value is {}", rendering_distance,
                 m_rendering_distance.load());
    request_chunk();
}

int ClientWorld::get_chunk_task_id() const { return m_chunk_task_id.load(); }

const std::vector<const ChunkRenderSnapshot*>&
ClientWorld::render_snapshots() const {
    return m_render_snapshots;
};
const std::vector<PlayerRenderData>& ClientWorld::render_player_data() const {
    return m_render_player_data;
}
std::vector<PlayerRenderData>& ClientWorld::render_player_data() {
    return m_render_player_data;
}
std::vector<glm::vec4>& ClientWorld::planes() { return m_planes; }
} // namespace Cubed