#pragma once

#include "Cubed/config.hpp"
#include "Cubed/gameplay/cave_carver.hpp"
#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/packet.hpp" // IWYU pragma: keep
#include "Cubed/gameplay/river_worm.hpp"
#include "Cubed/gameplay/server_chunk.hpp"
#include "Cubed/gameplay/server_entity_manager.hpp"
#include "Cubed/gameplay/server_player.hpp"
#include "Cubed/gameplay/world.hpp"
#include "Cubed/tools/priority_thread_pool.hpp"
#include "Cubed/tools/recent_queue.hpp"
#include "Cubed/tools/sensitive_filter.hpp"
#include "Cubed/tools/thread_pool.hpp"
#include "Cubed/ui/color.hpp"
#include "world/block_change.pb.h"

#include <absl/container/flat_hash_set.h>
#include <shared_mutex>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <unordered_map>
#include <utility>
#include <vector>
namespace Cubed {
class Session;
class ServerWorld : public World {
public:
    enum class ThreadPoolKind { NET, GEN };
    ServerWorld(Config& config);
    ~ServerWorld();
    void stop();
    void handle_player_exit(const std::string& uuid);
    void init_world(RunMode mode);
    void need_gen(std::string uuid);
    void update();
    void hot_reload();

    int rendering_distance() const;
    void rendering_distance(int rendering_distance);
    void start_gen_thread();
    void start_server_thread();

    void stop_gen_thread();
    void stop_server_thread();

    void stop_thread_pool();
    void start_thread_pool();

    void serever_run(std::stop_token stoken);

    CaveCarver& cave_carcer();
    RiverWorm& river_worm();

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

    bool set_block(const glm::ivec3& block_pos, unsigned id);

    void sync_player_pos(const C2S_PlayerInfo& rsp);
    void sync_player_water_sound(const PlayerWaterSound& rsp);
    void handle_player_login(const std::string& player_name,
                             std::shared_ptr<Session> session);
    glm::vec3 get_player_pos(const std::string& uuid) const;

    void handle_chunk_req(int task_id, const std::string& uuid, ChunkPos pos);
    void handle_block_change(const BlockChangeReq& req);

    void handle_chat_message(ChatMsg& msg);
    void handle_voice_message(VoiceMsg& msg);

    void handle_entity_create(C2SEntityCreateRequest& req);
    void handle_entity_destory(C2SEntityDestoryRequest& req);

    int chunk_size() const;

    tbb::concurrent_vector<std::shared_ptr<Session>> get_all_session() const;

    uint32_t get_chunk_ref_count(const glm::vec3& pos) const;
    ServerEntityManager& entity_manager();
    std::shared_ptr<ThreadPool> get_compute_pool();
    size_t player_sum() const;

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
    enum class ChunkState { NONE, GENERATING, READY, PENDING_DELETE };
    struct ChunkEntity {
        ChunkState state;
        std::shared_ptr<ServerChunk> chunk;
        std::atomic<uint32_t> ref_count = 0;
        ChunkEntity() = default;
        ChunkEntity(ChunkState s, std::shared_ptr<ServerChunk> c = {})
            : state(s), chunk(std::move(c)) {}

        ChunkEntity& operator=(ChunkEntity&& o) noexcept {
            if (this == &o) {
                return *this;
            }

            state = std::exchange(o.state, ServerWorld::ChunkState::NONE);
            chunk = std::move(o.chunk);
            ref_count = o.ref_count.exchange(0);
            return *this;
        }

        ChunkEntity(ChunkEntity&& o) noexcept
            : state(std::exchange(o.state, ServerWorld::ChunkState::NONE)),
              chunk(std::move(o.chunk)), ref_count(o.ref_count.exchange(0)) {}
        ChunkEntity(const ChunkEntity&) = delete;
        ChunkEntity& operator=(const ChunkEntity&) = delete;
    };

    enum class ChunkLoadStyle { RANDOM, CENTER };
    struct PendingRequest {
        std::string uuid;
        int task_id;
        ChunkPos pos;
    };
    struct PendingChunk {
        ChunkPos pos;
        std::unique_ptr<ServerChunk> chunk;
    };

    using ChunkHashMap =
        tbb::concurrent_hash_map<ChunkPos, ChunkEntity, ChunkPos::TBBHash>;
    using PlayerHashMap = std::unordered_map<std::string, ServerPlayer>;
    using NewChunkVector = std::vector<PendingChunk>;
    using ChunkPosSet = absl::flat_hash_set<ChunkPos, ChunkPos::Hash>;
    using PlayerUUIDMap = tbb::concurrent_hash_map<std::string, std::string>;

    using chunk_acc = ChunkHashMap::accessor;
    using chunk_cacc = ChunkHashMap::const_accessor;

    using uuid_acc = PlayerUUIDMap::accessor;
    using uuid_cacc = PlayerUUIDMap::const_accessor;

    Config& m_config;
    std::atomic<RunMode> m_runmode{RunMode::HYBRID};
    ServerEntityManager m_entity_manager;
    // key = uuid
    PlayerHashMap m_players;
    ChunkHashMap m_chunks;

    CaveCarver m_cave_carcer;
    RiverWorm m_river_worm;
    std::atomic<bool> m_enable_filter{false};
    std::atomic<bool> m_voice_chat{true};
    SensitiveFilter m_filter;

    std::jthread m_gen_thread;
    std::jthread m_server_thread;

    std::atomic<bool> m_chunk_gen_finished{false};
    std::atomic<bool> m_could_gen{true};
    std::atomic<bool> m_gen_running{false};
    std::atomic<bool> m_need_gen_chunk{false};
    std::atomic<bool> m_init{false};
    std::atomic<bool> m_stopped{false};
    std::atomic<int> m_rendering_distance{24};
    std::atomic<int> m_gen_threads{0};
    std::atomic<int> m_net_threads{0};
    std::atomic<int> m_compute_threads{0};
    std::atomic<int> m_max_threads{1};
    std::atomic<size_t> m_player_sum{0};
    std::atomic<TickType> m_game_ticks{0};
    std::atomic<TickType> m_day_tick{6000};
    std::atomic<bool> m_tick_running{true};
    std::atomic<int> m_per_tick_time = DEFAULT_PER_TICK_TIME; // ms

    mutable std::shared_mutex m_players_mutex;
    std::mutex m_need_gen_queue_mutex;
    std::condition_variable_any m_gen_cv;

    RecentQueue<std::string> m_need_gen_queue;

    std::atomic<std::shared_ptr<PriorityThreadPool>> m_gen_thread_pool;
    std::atomic<std::shared_ptr<ThreadPool>> m_net_thread_pool;
    std::atomic<std::shared_ptr<ThreadPool>> m_compute_thread_pool;

    std::atomic<ChunkLoadStyle> m_chunk_load_style{ChunkLoadStyle::CENTER};

    PlayerUUIDMap m_uuid_to_name;

    tbb::concurrent_unordered_map<std::string, TickTimer> m_timers;
    tbb::concurrent_queue<PendingRequest> m_waiting_chunk_requests;
    tbb::concurrent_queue<std::unique_ptr<ServerChunk>> m_finished_queue;

    void init_chunks();

    void gen_chunks_internal(const std::string& uuid);

    void compute_required_chunks(ChunkPosSet& required_chunks,
                                 const std::optional<std::string>& uuid);
    void sync_and_collect_missing_chunks(std::vector<ChunkPos>&,
                                         const ChunkPosSet&);
    void submit_new_chunks(const std::string& uuid, NewChunkVector& new_chunks);
    // void wait_all_chunk_tasks();

    void update_ref_count(const ChunkPosSet& old, const ChunkPosSet& now);

    void send_time();

    void send_chunk(int task_id, const std::string& uuid, ChunkPos pos);

    int
    change_pool_threads(std::atomic<std::shared_ptr<ThreadPool>>& thread_pool,
                        int threads);
    int change_pool_threads(
        std::atomic<std::shared_ptr<PriorityThreadPool>>& thread_pool,
        int threads);
    void send_server_stop();

    void boardcast_message(const std::string& name, const std::string& message,
                           Color color = Color::WHITE, bool system_msg = false);
};
} // namespace Cubed
