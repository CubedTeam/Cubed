#include "Cubed/gameplay/server/server_world.hpp"

#include "Cubed/gameplay/packet.hpp"
#include "Cubed/gameplay/server/session.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/math_tools.hpp"
#include "Cubed/tools/proto_utils.hpp"
#include "Cubed/tools/system_time_utils.hpp"
#include "Cubed/tools/threas_utils.hpp"
#include "Cubed/tools/uuid.hpp"
#include "Cubed/tools/world_name.hpp"

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
    m_players_manager.stop();
    m_entity_manager.stop();
    m_chunk_system.stop();

    try {
        save_metadata();
    } catch (const std::exception& e) {
        Logger::error("Save Metadata Error {}", e.what());
    }
}

void ServerWorld::send_time() {
    Arena arena;
    auto* rsp = Arena::Create<UpdateTime>(&arena);

    rsp->set_day_tick(m_day_ticks);
    rsp->set_game_tick(m_game_ticks);
    auto sessions = get_all_session();
    auto packet = make_packet(*rsp);
    for (auto& s : sessions) {
        s->send(packet, 3);
    }
}

void ServerWorld::load_metadata(std::optional<uint32_t> seed) {
    auto value = m_storage->get_metadata();
    Document doc;
    if (value && Tools::parse_json_from_string(doc, *value)) {

        if (Tools::get_json_value(doc, "version", m_metadata.version)) {
            if (m_metadata.version > WorldStorage::VERSION) {
                throw std::runtime_error("Local MetaData Version is too low");
            }
        }

        if (!Tools::get_json_value(doc, "seed", m_metadata.seed)) {

            if (seed) {
                ChunkGenerator::init(*seed);
            } else {
                ChunkGenerator::init();
            }

            m_metadata.seed = ChunkGenerator::seed();
        } else {
            ChunkGenerator::init(m_metadata.seed);
        }

        if (Tools::get_json_value(doc, "game_ticks", m_metadata.game_ticks)) {
            m_game_ticks = m_metadata.game_ticks;
        }

        if (Tools::get_json_value(doc, "day_ticks", m_metadata.day_ticks)) {
            m_day_ticks = m_metadata.day_ticks % DAY_TIME;
        }
        EntityID next = 0;
        if (Tools::get_json_value(doc, "entity_next", next)) {
            m_entity_manager.set_next_value(next);
        }

    } else {
        if (seed) {
            ChunkGenerator::init(*seed);
        } else {
            ChunkGenerator::init();
        }
        m_metadata.seed = ChunkGenerator::seed();
    }

    save_metadata();
}

void ServerWorld::save_metadata() {

    Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();
    doc.AddMember("version", m_metadata.version, allocator);
    doc.AddMember("seed", m_metadata.seed, allocator);
    doc.AddMember("game_ticks", m_game_ticks.load(), allocator);
    doc.AddMember("day_ticks", m_day_ticks.load(), allocator);
    doc.AddMember("entity_next", m_entity_manager.get_next_value(), allocator);
    std::string value = Tools::to_json_string(doc);
    m_storage->save_metadata(value);
}

void ServerWorld::init_world(RunMode mode, std::string_view world_name,
                             std::optional<uint32_t> seed) {
    if (!Tools::is_valid_world_name(world_name)) {
        throw std::invalid_argument("Invalid world name");
    }

    const fs::path SAVE_PATH = SAVE_ROOT / std::string(world_name);
    if (SAVE_PATH.lexically_normal().parent_path() !=
        SAVE_ROOT.lexically_normal()) {
        throw std::invalid_argument("World path escapes save directory");
    }

    m_storage = std::make_unique<WorldStorage>(SAVE_PATH);

    load_metadata(seed);
    m_runmode = mode;
    m_players_manager.init();
    m_entity_manager.init();
    m_chunk_system.initialize();
    register_timer("player disconnect", 5, [this]() {
        std::vector<std::shared_ptr<ServerPlayer>> disconnect;
        auto players = m_players_manager.snapshot();
        if (!players) {
            return;
        }
        for (auto& [_, player] : *players) {
            if (player->is_disconnect(m_game_ticks)) {
                disconnect.emplace_back(player);
            }
        }

        for (auto& player : disconnect) {
            handle_player_exit(player);
        }
    });
    // Periodically process pending players
    register_timer("player chunk send", 1,
                   [this]() { m_chunk_system.pop_pending_request(); });
    // 5min(50ms)
    register_timer("auto save world", 6000, [this]() {
        try {
            m_players_manager.save_all();
            m_chunk_system.save_all_chunks();
            m_entity_manager.save_all_entities(false);
            save_metadata();
        } catch (const std::exception& e) {
            Logger::error("Save World Error {}", e.what());
        }
    });

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

    Logger::info("Server world initialization successful.");
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
            m_day_ticks = (m_day_ticks + 1) % DAY_TIME;
        }
        update();
        std::this_thread::sleep_until(next);
    }
    Logger::info("Server Thread Stopped!");
}

void ServerWorld::request_generation(Uuid uuid) {
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

    m_chunk_system.update();
    m_entity_manager.update();

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
    auto uuid = Uuid::from_proto_bytes(prsp.uuid());
    if (!uuid) {
        Logger::error("Can't parse uuid from proto");
        return;
    }
    auto yaw = prsp.yaw();
    auto pitch = prsp.pitch();

    auto player = m_players_manager.find(*uuid);
    if (!player) {
        Logger::warn("Player {} is not in this Server", uuid->to_string());
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
        if (o_uuid == *uuid) {
            continue;
        }
        if (player->has_player(c_pos)) {
            other.emplace_back(player->get_session());
        }
    }

    Arena arena;
    auto* rsp = Arena::Create<PlayerInfoRsp>(&arena);
    rsp->set_uuid(uuid->to_proto_bytes());
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
    auto uuid = Uuid::from_proto_bytes(rsp.uuid());
    if (!uuid) {
        Logger::error("Can't parse uuid from proto");
        return;
    }
    auto underwater = rsp.underwater();

    std::vector<std::shared_ptr<Session>> other;
    auto players = m_players_manager.snapshot();

    if (!players) {
        Logger::error("Can't get players map");
        return;
    }

    for (auto& [o_uuid, player] : *players) {
        if (o_uuid == *uuid) {
            continue;
        }
        if (player->has_player(pos)) {
            other.emplace_back(player->get_session());
        }
    }

    Arena arena;
    auto* r = Arena::Create<PlayerWaterSound>(&arena);
    r->set_uuid(uuid->to_proto_bytes());
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

void ServerWorld::send_player_login_error(int32_t ec, std::string_view err_msg,
                                          std::shared_ptr<Session> session) {
    Arena arena;
    auto msg = Arena::Create<LoginRsp>(&arena);
    auto err = msg->mutable_error();
    err->set_mes(err_msg);
    err->set_code(ec);
    session->send(make_packet(msg));
}

void ServerWorld::handle_player_login(LoginReq& msg,
                                      std::shared_ptr<Session> session) {

    auto uuid = Uuid::from_proto_bytes(msg.uuid());
    if (!uuid) {
        send_player_login_error(
            1, std::format("Can't parse uuid {}", msg.uuid()), session);
        return;
    }
    std::optional<Crypto::Ed25519PublicKey> pk =
        Crypto::Ed25519::public_key_from_proto_bytes(msg.public_key());
    if (!pk) {
        send_player_login_error(1, "Can't parse public key ", session);
        return;
    }
    auto uuid_pk = Crypto::Ed25519::uuid_from_public_key(*pk);
    if (*uuid != uuid_pk) {
        send_player_login_error(1,
                                std::format("uuid {} from net is not equal "
                                            "with uuid {} from public compute",
                                            uuid->to_string(),
                                            uuid_pk.to_string()),
                                session);

        return;
    }

    auto storage = m_players_manager.get_storage();
    auto player_data = storage->load(*uuid);
    if (player_data) {
        if (player_data->uuid != *uuid || player_data->public_key != *pk) {
            send_player_login_error(
                1,
                std::format("uuid {} and pk not equal with database",
                            uuid->to_string()),
                session);

            return;
        }
    }

    session->public_key() = *pk;
    std::pair<uint64_t, Crypto::Ed25519::Challenge> challenge;
    challenge.second = Crypto::Ed25519::generate_challenge();
    challenge.first = Tools::get_utc_timestamp_ms();
    session->challenge() = challenge;

    Arena arena;
    auto* p = Arena::Create<LoginChallenge>(&arena);

    p->set_challenge(challenge.second.data(), challenge.second.size());
    p->set_uuid(uuid->to_proto_bytes());

    session->send(make_packet(p), 0);

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
    /*
     */
}

void ServerWorld::handle_ping(Ping& ping, std::shared_ptr<Session> session) {
    Arena arena;
    auto* msg = Arena::Create<Pong>(&arena);
    msg->set_timestamp(ping.timestamp());
    session->send(make_packet(msg), 0);
}

void ServerWorld::handle_login_proof(LoginProof& msg,
                                     std::shared_ptr<Session> session) {

    auto s = msg.signature();
    if (s.size() != 64) {
        send_player_login_error(1, "Invailed signature", session);

        return;
    }
    if (!session->challenge()) {
        send_player_login_error(1, "Challenge is null", session);

        return;
    }
    if (Tools::get_utc_timestamp_ms() - session->challenge()->first >
        Session::CHALLENGE_EXPIRE_MS) {
        send_player_login_error(1, "Challenge expire", session);
        session->challenge().reset();
        return;
    }

    Crypto::Ed25519Signature sign;
    std::copy_n(reinterpret_cast<const unsigned char*>(s.data()), s.size(),
                sign.data.begin());

    auto signing_message = Crypto::Ed25519::make_login_signing_data(
        session->challenge()->second, session->public_key());

    session->challenge().reset();

    if (!Crypto::Ed25519::verify(signing_message, sign,
                                 session->public_key())) {
        send_player_login_error(1, "Can't verify signature", session);

        return;
    }

    auto name = msg.name();

    auto u = Uuid::from_proto_bytes(msg.uuid());
    if (!u) {
        send_player_login_error(1, "Can't parse uuid from proto", session);

        return;
    }

    Uuid uuid = Crypto::Ed25519::uuid_from_public_key(session->public_key());

    if (uuid != *u) {
        send_player_login_error(
            1, "uuid from proto is not equal with uuid from public", session);
        return;
    }

    auto p = m_players_manager.find(uuid);
    if (p && p->is_disconnect(m_game_ticks)) {
        handle_player_exit(p, true);
    }

    Logger::info("Player {} (uuid {}) join the world", name, uuid.to_string());

    auto player = std::make_shared<ServerPlayer>(name, uuid, *this, session,
                                                 m_game_ticks);

    bool inserted = m_players_manager.add(player);
    if (!inserted) {

        send_player_login_error(1, "Player already in this server", session);

        return;
    }
    session->set_player_uuid(uuid);
    Arena arena;

    auto* rsp = Arena::Create<LoginRsp>(&arena);
    auto err = rsp->mutable_error();
    err->set_code(0);
    rsp->set_voice_chat(m_voice_chat);
    rsp->set_pitch(player->pitch());
    rsp->set_yaw(player->yaw());
    Tools::set_proto_vec3(rsp->mutable_pos(), player->get_pos());

    session->send(make_packet(*rsp), 0);

    boardcast_message("Server", std::format("Player {} Join Game", name),
                      Color::YELLOW, true);

    request_generation(uuid);
    m_entity_manager.handle_player_login(session);

    return;
}

void ServerWorld::handle_player_exit(std::shared_ptr<ServerPlayer> player,
                                     bool sync) {
    if (!player) {
        return;
    }
    auto pool = m_net_thread_pool.load();
    if (!pool || sync) {
        player_exit(std::move(player));
        return;
    }
    pool->enqueue([this, player = std::move(player)]() mutable {
        player_exit(std::move(player));
    });
}

glm::vec3 ServerWorld::get_player_pos(const Uuid& uuid) const {
    auto pos = m_players_manager.get_position(uuid);

    if (!pos) {
        Logger::error("Can't find player uuid {}", uuid.to_string());
        return glm::vec3{0.0f};
    }

    return *pos;
}

void ServerWorld::handle_chunk_req(int task_id, const Uuid& uuid,
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

    auto uuid = Uuid::from_proto_bytes(msg.uuid());
    if (!uuid) {
        return;
    }
    std::string message = msg.msg();
    auto pool = m_net_thread_pool.load();
    pool->enqueue([this, player = *uuid, m = std::move(message)]() {
        auto p = m_players_manager.find(player);
        if (!p) {
            return;
        }
        boardcast_message(p->get_name(), m);
    });
}

void ServerWorld::handle_voice_message(VoiceMsg& msg) {
    ZoneScopedN("ServerWorld::handle_voice_message");
    if (!m_voice_chat) {
        return;
    }
    auto pool = m_net_thread_pool.load();
    auto uuid = Uuid::from_proto_bytes(msg.uuid());
    if (!uuid) {
        Logger::error("Can't parse uuid from proto");
        return;
    }
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
        msg->set_uuid(uuid->to_proto_bytes());

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

    m_entity_manager.add_creature(req.name(), Tools::get_proto_vec3(req.pos()));
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
TickType ServerWorld::day_tick() const { return m_day_ticks.load(); }
void ServerWorld::day_tick(TickType tick) {
    tick %= DAY_TIME;
    m_day_ticks = tick;
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

void ServerWorld::player_exit(std::shared_ptr<ServerPlayer> expected_player) {

    if (!expected_player) {
        return; // Already removed
    }
    const Uuid UUID = expected_player->get_uuid();
    auto player = m_players_manager.remove(UUID, expected_player);

    if (!player) {
        return;
    }
    auto exit_session = player->get_session();
    const std::string NAME = player->get_name();
    Logger::info("Player {} Exit the Server", NAME);

    m_chunk_system.release_chunk(player);

    Arena arena;
    auto* rsp = Arena::Create<LogoutRsp>(&arena);
    rsp->set_uuid(UUID.to_proto_bytes());
    rsp->set_server_stop(false);
    exit_session->send(make_packet(*rsp), 0);
    exit_session->set_player_uuid(std::nullopt);
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

bool ServerWorld::is_chunk_active(glm::vec3 pos) const {
    auto chunk_pos = get_chunk_pos(pos.x, pos.z);
    return m_chunk_system.is_chunk_active(chunk_pos);
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
WorldStorage* ServerWorld::world_storage() { return m_storage.get(); }
ServerChunkSystem& ServerWorld::chunk_system() { return m_chunk_system; }

std::shared_ptr<ThreadPool> ServerWorld::get_compute_pool() {
    return m_compute_thread_pool.load();
}

size_t ServerWorld::player_sum() const { return m_players_manager.sum(); }
RunMode ServerWorld::get_runmode() const { return m_runmode; }
} // namespace Cubed
