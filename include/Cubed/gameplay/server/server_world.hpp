#pragma once

#include "Cubed/config.hpp"
#include "Cubed/gameplay/cave_carver.hpp"
#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/packet.hpp" // IWYU pragma: keep
#include "Cubed/gameplay/river_worm.hpp"
#include "Cubed/gameplay/server/server_chunk.hpp"
#include "Cubed/gameplay/server/server_chunk_system.hpp"
#include "Cubed/gameplay/server/server_entity_manager.hpp"
#include "Cubed/gameplay/server/server_player.hpp"
#include "Cubed/gameplay/server/server_player_manager.hpp"
#include "Cubed/gameplay/server/world_storage.hpp"
#include "Cubed/gameplay/world.hpp"
#include "Cubed/tools/priority_thread_pool.hpp"
#include "Cubed/tools/sensitive_filter.hpp"
#include "Cubed/tools/thread_pool.hpp"
#include "Cubed/ui/color.hpp"
#include "world/block_change.pb.h"

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
namespace Cubed {
class Session;
class ServerWorld : public World {
public:
    static inline const std::filesystem::path SAVE_ROOT = "./saves";
    struct Metadata {
        uint32_t version = WorldStorage::VERSION;
        uint32_t seed = 0;
        TickType game_ticks = 0;
        TickType day_ticks = 0;
    };

    enum class ThreadPoolKind { NORMAL, PRIORITY };
    ServerWorld(Config& config);
    ~ServerWorld();
    void stop();
    void handle_player_exit(std::shared_ptr<ServerPlayer> player,
                            bool sync = false);
    void init_world(RunMode mode, std::string_view world_name,
                    std::optional<uint32_t> seed);
    void request_generation(Uuid uuid);
    void update();
    void hot_reload();

    int rendering_distance() const;
    void rendering_distance(int rendering_distance);

    void start_server_thread();

    void stop_server_thread();

    void stop_thread_pool();
    void start_thread_pool();

    void serever_run(std::stop_token stoken);

    CaveCarver& cave_carcer();
    RiverWorm& river_worm();

    Config& get_config();

    TickType game_tick() const;
    TickType day_tick() const;

    void day_tick(TickType tick);

    int per_tick_time() const;
    void per_tick_time(int ms);
    bool is_tick_running() const;
    void tick_running(bool run);

    int gen_pool_threads() const;
    int max_threads() const;

    void change_pool_threads(ThreadPoolKind kind, int threads);

    int chunk_load_style() const;
    void set_chunk_load_style(int id);

    bool set_block(const glm::ivec3& block_pos, BlockType id);

    void sync_player_pos(const C2S_PlayerInfo& rsp);
    void sync_player_water_sound(const PlayerWaterSound& rsp);
    void handle_player_login(LoginReq& msg, std::shared_ptr<Session> session);
    void handle_login_proof(LoginProof& msg, std::shared_ptr<Session> session);
    void handle_ping(Ping& ping, std::shared_ptr<Session> session);
    void send_player_login_error(int32_t ec, std::string_view msg,
                                 std::shared_ptr<Session> session);
    glm::vec3 get_player_pos(const Uuid& uuid) const;
    void handle_chunk_req(int task_id, const Uuid& uuid, ChunkPos pos);
    void handle_block_change(const BlockChangeReq& req);

    void handle_chat_message(ChatMsg& msg);
    void handle_voice_message(VoiceMsg& msg);

    void handle_entity_create(C2SEntityCreateRequest& req);
    void handle_entity_destory(C2SEntityDestoryRequest& req);

    int chunk_size() const;

    tbb::concurrent_vector<std::shared_ptr<Session>> get_all_session() const;

    uint32_t get_chunk_ref_count(const glm::vec3& pos) const;
    ServerEntityManager& entity_manager();
    ServerPlayerManager& player_manager();
    WorldStorage* world_storage();
    ServerChunkSystem& chunk_system();

    std::shared_ptr<ThreadPool> get_compute_pool();
    size_t player_sum() const;
    RunMode get_runmode() const;
    bool is_chunk_active(glm::vec3 pos) const;

    int get_block(const glm::ivec3& block_pos) const override;
    bool is_solid(const glm::ivec3& block_pos) const override;
    bool can_pass_block(const glm::ivec3& block_pos) const override;
    BlockType get_block_tpye(const glm::ivec3& block_pos) const override;
    int get_per_tick_time() const override;

    template <typename Fn>
    void register_timer(std::string_view id, TickType threshold, Fn&& f) {
        m_timers.emplace(std::piecewise_construct,
                         std::forward_as_tuple(std::string(id)),
                         std::forward_as_tuple(threshold, std::forward<Fn>(f)));
    }

private:
    Config& m_config;
    std::unique_ptr<WorldStorage> m_storage;
    Metadata m_metadata;

    std::atomic<RunMode> m_runmode{RunMode::HYBRID};
    ServerEntityManager m_entity_manager;
    ServerPlayerManager m_players_manager;

    ServerChunkSystem m_chunk_system;

    CaveCarver m_cave_carcer;
    RiverWorm m_river_worm;
    std::atomic<bool> m_enable_filter{false};
    std::atomic<bool> m_voice_chat{true};
    SensitiveFilter m_filter;

    std::jthread m_server_thread;

    std::atomic<bool> m_init{false};
    std::atomic<bool> m_stopped{false};

    std::atomic<int> m_net_threads{0};
    std::atomic<int> m_compute_threads{0};
    std::atomic<int> m_max_threads{1};

    std::atomic<TickType> m_game_ticks{0};
    std::atomic<TickType> m_day_ticks{6000};
    std::atomic<bool> m_tick_running{true};
    std::atomic<int> m_per_tick_time = DEFAULT_PER_TICK_TIME; // ms

    std::atomic<std::shared_ptr<ThreadPool>> m_net_thread_pool;
    std::atomic<std::shared_ptr<ThreadPool>> m_compute_thread_pool;

    tbb::concurrent_unordered_map<std::string, TickTimer> m_timers;

    void init_chunks();

    void send_time();

    int
    change_pool_threads(std::atomic<std::shared_ptr<ThreadPool>>& thread_pool,
                        int threads);
    int change_pool_threads(
        std::atomic<std::shared_ptr<PriorityThreadPool>>& thread_pool,
        int threads);
    void send_server_stop();

    void boardcast_message(const std::string& name, const std::string& message,
                           Color color = Color::WHITE, bool system_msg = false);
    void player_exit(std::shared_ptr<ServerPlayer> expected_player);

    void load_metadata(std::optional<uint32_t> seed);
    void save_metadata();
};
} // namespace Cubed
