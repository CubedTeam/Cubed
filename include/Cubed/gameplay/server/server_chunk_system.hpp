#pragma once

#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/server/server_chunk.hpp"
#include "Cubed/tools/priority_thread_pool.hpp"
#include "Cubed/tools/recent_queue.hpp"

#include <absl/container/flat_hash_set.h>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <optional>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <utility>
#include <vector>
namespace Cubed {

class Config;
class CaveCarver;
class RiverWorm;
class ServerPlayerManager;
class ServerWorld;
class ServerPlayer;
class ServerChunkSystem {
public:
    struct ChunkRequest {
        std::string uuid;
        int task_id;
        ChunkPos pos;
    };

    enum class ChunkState { NONE, GENERATING, READY, GENERATING_UNUSED };
    struct ChunkEntity {
        ChunkState state = ChunkState::NONE;
        std::shared_ptr<ServerChunk> chunk;
        std::atomic<uint32_t> ref_count = 0;

        uint64_t generation_id = 0;

        ChunkEntity() = default;
        ChunkEntity(ChunkState s, std::shared_ptr<ServerChunk> c = {})
            : state(s), chunk(std::move(c)) {}

        ChunkEntity& operator=(ChunkEntity&& o) noexcept {
            if (this == &o) {
                return *this;
            }

            state = std::exchange(o.state, ChunkState::NONE);
            chunk = std::move(o.chunk);
            ref_count = o.ref_count.exchange(0);
            generation_id = o.generation_id;
            return *this;
        }

        ChunkEntity(ChunkEntity&& o) noexcept
            : state(std::exchange(o.state, ChunkState::NONE)),
              chunk(std::move(o.chunk)), ref_count(o.ref_count.exchange(0)),
              generation_id(o.generation_id) {}
        ChunkEntity(const ChunkEntity&) = delete;
        ChunkEntity& operator=(const ChunkEntity&) = delete;
    };

    struct GenerationTicket {
        ChunkPos pos;
        uint64_t generation_id;
    };

    struct PendingChunk {
        ChunkPos pos;
        uint64_t generation_id;
        std::unique_ptr<ServerChunk> chunk;
    };

    struct FinishedChunk {
        ChunkPos pos;
        uint64_t generation_id;
        std::unique_ptr<ServerChunk> chunk;
    };

    using NewChunkVector = std::vector<PendingChunk>;

    ServerChunkSystem(ServerWorld& world);

    ~ServerChunkSystem();

    ServerChunkSystem(const ServerChunkSystem&) = delete;
    ServerChunkSystem& operator=(const ServerChunkSystem&) = delete;

    void initialize();
    void stop();

    void update();

    [[nodiscard]] int get_block(const glm::ivec3& pos) const;
    [[nodiscard]] bool is_solid(const glm::ivec3& pos) const;
    [[nodiscard]] bool can_pass_block(const glm::ivec3& pos) const;
    [[nodiscard]] BlockType get_block_type(const glm::ivec3& pos) const;

    bool set_block(const glm::ivec3& pos, BlockType id);

    [[nodiscard]]
    uint32_t get_chunk_ref_count(ChunkPos pos) const;

    void set_render_distance(int distance);
    [[nodiscard]] int render_distance() const;

    void set_load_style(int id);
    [[nodiscard]] ChunkLoadStyle load_style() const;

    [[nodiscard]] int generation_threads() const;

    [[nodiscard]] bool is_chunk_active(ChunkPos pos) const;

    size_t chunk_size() const;

    void send_chunk(int task_id, const std::string& uuid, ChunkPos pos);

    void pop_pending_request();

    void start_gen_thread();

    void stop_gen_thread();

    void stop_generation_pool();

    void request_generation(const std::string& uuid);

    void hot_reload();

    void release_chunk(const std::shared_ptr<ServerPlayer>& player);

    void save_all_chunks(bool sync = false);
    std::vector<ChunkStorageData> copy_all_chunks() const;

    ChunkStorage* get_storage();

private:
    using ChunkHashMap =
        tbb::concurrent_hash_map<ChunkPos, ChunkEntity, ChunkPos::TBBHash>;
    using chunk_acc = ChunkHashMap::accessor;
    using chunk_cacc = ChunkHashMap::const_accessor;

    ServerWorld& m_world;

    std::unique_ptr<ChunkStorage> m_storage;

    ChunkHashMap m_chunks;

    std::jthread m_generation_scheduler;

    std::atomic<bool> m_generation_running{false};
    std::atomic<bool> m_need_generation{false};
    std::atomic<bool> m_could_generate{true};
    std::atomic<bool> m_chunk_gen_finished{false};
    std::atomic<bool> m_scheduler_alive{false};
    std::mutex m_generation_queue_mutex;

    mutable std::mutex m_reconcile_mutex;
    std::condition_variable_any m_generation_cv;
    RecentQueue<std::string> m_generation_queue;

    std::atomic<std::shared_ptr<PriorityThreadPool>> m_generation_pool;
    std::atomic<int> m_generation_threads{0};

    std::atomic<uint64_t> m_next_generation_id{1};

    std::atomic<int> m_render_distance{24};
    std::atomic<ChunkLoadStyle> m_load_style{ChunkLoadStyle::CENTER};

    tbb::concurrent_queue<ChunkRequest> m_waiting_requests;
    tbb::concurrent_queue<FinishedChunk> m_finished_chunks;

    void gen_chunks_internal(const std::string& uuid);

    void compute_required_chunks(ChunkPosSet& required_chunks,
                                 const std::optional<std::string>& uuid);

    void submit_new_chunks(const std::string& uuid, NewChunkVector& new_chunks);

    void reconcile_chunks(const ChunkPosSet& old_set,
                          const ChunkPosSet& new_set,
                          std::vector<GenerationTicket>& new_generations);

    // void wait_all_chunk_tasks();
};
} // namespace Cubed