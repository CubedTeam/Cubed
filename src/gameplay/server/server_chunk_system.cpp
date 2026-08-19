#include "Cubed/gameplay/server/server_chunk_system.hpp"

#include "Cubed/gameplay/block_manager.hpp"
#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/gameplay/server/session.hpp"
#include "Cubed/tools/threas_utils.hpp"

#include <tracy/Tracy.hpp>
using namespace google::protobuf;
namespace fs = std::filesystem;
namespace cubed {
ServerChunkSystem::ServerChunkSystem(ServerWorld& world) : m_world(world) {}

ServerChunkSystem::~ServerChunkSystem() {}

void ServerChunkSystem::initialize() {

    m_storage = std::make_unique<ChunkStorage>(*m_world.world_storage());

    auto gen_threads = tools::get_server_gen_threads(m_world.get_runmode());
    Logger::info("Server Gen pool threads {}", gen_threads);
    m_generation_threads = gen_threads;
    m_generation_pool.store(std::make_shared<PriorityThreadPool>(gen_threads));
    Logger::info("ServerChunkSystem initialization successful.");
}

void ServerChunkSystem::stop() {
    stop_gen_thread();
    stop_generation_pool();

    save_all_chunks(true);
    m_finished_chunks.clear();
    m_chunks.clear();
    Logger::info("ServerChunkSystem stopped successful.");
}

void ServerChunkSystem::save_all_chunks(bool sync) {

    auto chunks = copy_all_chunks();

    if (chunks.empty()) {
        return;
    }
    if (sync) {
        if (m_storage->save_batch(chunks, true)) {
            Logger::info("Save all chunks success!");
        }

    } else {
        auto pool = m_world.get_compute_pool();

        if (!pool) {
            if (m_storage->save_batch(chunks)) {
                Logger::info("Save all chunks success!");
            }

            return;
        }

        pool->enqueue([this, chunks = std::move(chunks)]() mutable {
            ZoneScopedN("Save All Chunk Batch");
            if (m_storage->save_batch(chunks)) {
                Logger::info("Save all chunks success!");
            }
        });
    }
}

std::vector<ChunkStorageData> ServerChunkSystem::copy_all_chunks() const {
    ZoneScopedN("ServerChunkSystem::copy_all_chunks");
    std::vector<ChunkPos> positions;

    {
        std::lock_guard lock(m_reconcile_mutex);

        positions.reserve(m_chunks.size());

        for (const auto& [pos, _] : m_chunks) {
            positions.push_back(pos);
        }
    }

    std::vector<ChunkStorageData> chunks;
    chunks.reserve(positions.size());

    for (const ChunkPos& pos : positions) {
        chunk_cacc acc;

        if (!m_chunks.find(acc, pos)) {
            continue;
        }

        const ChunkEntity& entity = acc->second;

        if (entity.state != ChunkState::READY || !entity.chunk) {
            continue;
        }

        chunks.emplace_back(entity.chunk->make_storage_data());
    }

    return chunks;
}

void ServerChunkSystem::update() {
    ZoneScopedN("ServerChunkSystem::update");
    bool consumed = false;
    FinishedChunk finished;
    while (m_finished_chunks.try_pop(finished)) {
        if (!finished.chunk) {
            Logger::error("Finished Queue has nullptr Chunk");
            continue;
        }

        std::shared_ptr<ServerChunk> ready_chunk;

        {
            chunk_acc acc;

            if (!m_chunks.find(acc, finished.pos)) {
                Logger::error(
                    "New Chunk {} {} not Find, don't move to m_chunks",
                    finished.pos.x, finished.pos.z);
                continue;
            }

            if (acc->second.generation_id != finished.generation_id) {
                Logger::warn(
                    "Discard stale chunk result {} {}, generation {} != {}",
                    finished.pos.x, finished.pos.z, finished.generation_id,
                    acc->second.generation_id);

                continue;
            }
            if (acc->second.state == ChunkState::GENERATING_UNUSED ||
                acc->second.ref_count.load(std::memory_order_relaxed) == 0) {

                m_chunks.erase(acc);
                continue;
            }

            if (acc->second.state != ChunkState::GENERATING) {
                // Prevent duplicate completion results from overwriting READY
                // chunks
                Logger::warn("Discard duplicate chunk result {} {}",
                             finished.pos.x, finished.pos.z);

                continue;
            }

            acc->second.chunk = std::move(finished.chunk);
            acc->second.state = ChunkState::READY;
            ready_chunk = acc->second.chunk;
            consumed = true;
            pop_pending_request();
        }
        ready_chunk->finished_generating();
        m_world.entity_manager().activate_chunk(finished.pos);
    }

    if (consumed) {
        m_could_generate = true;
    }
}

void ServerChunkSystem::release_chunk(
    const std::shared_ptr<ServerPlayer>& player) {

    std::lock_guard lock(m_reconcile_mutex);

    ChunkPosSet old_set = player->take_chunk_pos_set();

    std::vector<GenerationTicket> unused;
    reconcile_chunks(old_set, ChunkPosSet{}, unused);
}

void ServerChunkSystem::send_chunk(int task_id, const Uuid& uuid,
                                   ChunkPos pos) {

    ZoneScopedN("ServerChunkSystem::send_chunk");
    auto& m_players_manager = m_world.player_manager();
    auto m_game_ticks = m_world.game_tick();
    {
        auto id = m_players_manager.get_task_id(uuid);
        if (id == -1) {
            return;
        }
        if (task_id < id) {
            // Old chunk requests are simply discarded
            return;
        }
    }
    auto player = m_players_manager.find(uuid);
    if (!player) {
        Logger::error("Can't get player {}", uuid.to_string());
        return;
    }
    player->update_sync_gametick(m_game_ticks);
    std::shared_ptr<Session> s = player->get_session();
    if (!s) {
        Logger::error("Player {} session not exist", uuid.to_string());
        return;
    }
    Arena arean;
    protocol::S2CChunkDataRsp* rsp =
        Arena::Create<protocol::S2CChunkDataRsp>(&arean);
    auto* rsq_pos = rsp->mutable_pos();
    rsq_pos->set_x(pos.x);
    rsq_pos->set_z(pos.z);
    {
        chunk_cacc cacc;
        if (!m_chunks.find(cacc, pos)) {
            // No chunk found and not generating
            Logger::error("Chunk {} {} neither pending nor ready", pos.x,
                          pos.z);
            return;
        }

        if (cacc->second.state == ChunkState::GENERATING) {

            m_waiting_requests.emplace(uuid, task_id, pos);
            return;
        }
        if (cacc->second.state != ChunkState::READY) {
            Logger::error("Chunk {} {} is invaild", pos.x, pos.z);
            return;
        }

        rsp->set_chunk_seed(cacc->second.chunk->seed());
        rsp->set_biome_type(std::to_underlying(cacc->second.chunk->biome()));
        auto* blocks = rsp->mutable_chunk_blocks();
        auto chunk_blocks = cacc->second.chunk->get_chunk_blocks();
        blocks->Assign(chunk_blocks.begin(), chunk_blocks.end());
        auto& neighbor_blocks = cacc->second.chunk->get_neightbor_blocks();

        auto assign = [](auto* nb,
                         const std::optional<std::vector<BlockType>>& blocks) {
            if (!blocks) {
                return;
            }
            if (!nb) {
                return;
            }
            nb->Assign(blocks->begin(), blocks->end());
        };
        auto* nb1 = rsp->mutable_neighbor_blocks_one();
        auto* nb2 = rsp->mutable_neighbor_blocks_two();
        auto* nb3 = rsp->mutable_neighbor_blocks_three();
        auto* nb4 = rsp->mutable_neighbor_blocks_four();
        assign(nb1, neighbor_blocks[0]);
        assign(nb2, neighbor_blocks[1]);
        assign(nb3, neighbor_blocks[2]);
        assign(nb4, neighbor_blocks[3]);
    }

    rsp->set_task_id(task_id);
    s->send(make_packet(*rsp));
}

void ServerChunkSystem::pop_pending_request() {
    ChunkRequest request;
    if (m_waiting_requests.try_pop(request)) {
        m_world.handle_chunk_req(request.task_id, request.uuid, request.pos);
    }
}

void ServerChunkSystem::gen_chunks_internal(const Uuid& uuid) {
    ZoneScopedN("ServerChunkSystem::gen_chunks_internal");
    // Logger::info("gen_chunks_internal");
    m_chunk_gen_finished = false;

    ChunkPosSet required_chunks_set;

    compute_required_chunks(required_chunks_set, uuid);
    std::vector<GenerationTicket> new_generations;
    {
        std::lock_guard lock(m_reconcile_mutex);

        auto& m_players_manager = m_world.player_manager();

        auto player = m_players_manager.find(uuid);

        if (!player) {
            return;
        }

        ChunkPosSet old_set = player->get_chunk_pos_set();
        player->update_chunk_set(required_chunks_set);

        reconcile_chunks(old_set, required_chunks_set, new_generations);
    }

    NewChunkVector new_chunks;
    new_chunks.reserve(new_generations.size());
    // Create new chunk

    for (auto& ticket : new_generations) {
        new_chunks.push_back({
            .pos = ticket.pos,
            .generation_id = ticket.generation_id,
            .chunk = std::make_unique<ServerChunk>(m_world, ticket.pos),
        });
    }

    submit_new_chunks(uuid, new_chunks);
    m_chunk_gen_finished = true;
}

void ServerChunkSystem::compute_required_chunks(
    ChunkPosSet& required_chunks, const std::optional<Uuid>& uuid) {
    glm::vec3 player_pos;
    if (uuid == std::nullopt) {
        player_pos = glm::vec3{0.0f};
    } else {
        player_pos = m_world.get_player_pos(uuid.value());
    }
    int x = std::floor(player_pos.x);
    int z = std::floor(player_pos.z);
    auto [chunk_x, chunk_z] = get_chunk_pos(x, z);
    int radius = m_render_distance;
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

void ServerChunkSystem::reconcile_chunks(
    const ChunkPosSet& old_set, const ChunkPosSet& new_set,
    std::vector<GenerationTicket>& new_generations) {

    std::vector<ChunkEntity> m_removed_chunks;

    for (const auto& pos : old_set) {
        if (new_set.contains(pos)) {
            continue;
        }

        chunk_acc acc;
        if (!m_chunks.find(acc, pos)) {
            Logger::warn("Cannot find old chunk {} {}", pos.x, pos.z);
            continue;
        }

        const uint32_t PREVIOUS =
            acc->second.ref_count.fetch_sub(1, std::memory_order_relaxed);

        if (PREVIOUS == 0) {
            Logger::error("Chunk {} {} ref_count underflow", pos.x, pos.z);
            acc->second.ref_count.store(0);
            continue;
        }

        if (PREVIOUS != 1) {
            continue;
        }

        if (acc->second.state == ChunkState::GENERATING) {
            acc->second.state = ChunkState::GENERATING_UNUSED;
        } else {
            m_removed_chunks.emplace_back(std::move(acc->second));
            m_chunks.erase(acc);
        }
    }

    for (const auto& pos : new_set) {
        if (old_set.contains(pos)) {
            continue;
        }

        chunk_acc acc;
        const bool INSERTED = m_chunks.insert(acc, pos);

        if (INSERTED) {
            const uint64_t GENERATION_ID =
                m_next_generation_id.fetch_add(1, std::memory_order_relaxed);

            acc->second.state = ChunkState::GENERATING;
            acc->second.chunk.reset();
            acc->second.ref_count.store(1, std::memory_order_relaxed);
            acc->second.generation_id = GENERATION_ID;

            new_generations.push_back({
                .pos = pos,
                .generation_id = GENERATION_ID,
            });
        } else {
            acc->second.ref_count.fetch_add(1, std::memory_order_relaxed);

            if (acc->second.state == ChunkState::GENERATING_UNUSED) {

                acc->second.state = ChunkState::GENERATING;
            }
        }
    }

    auto save_removed_chunk = [this,
                               chunks = std::move(m_removed_chunks)]() mutable {
        std::vector<ChunkStorageData> data;
        for (auto& chunk : chunks) {
            data.emplace_back(chunk.chunk->make_storage_data());
        }
        chunks.clear();

        m_storage->save_batch(data);
    };

    save_removed_chunk();
}

void ServerChunkSystem::submit_new_chunks(const Uuid& uuid,
                                          NewChunkVector& new_chunks) {
    using enum ChunkLoadStyle;
    auto pool_ptr = m_generation_pool.load();
    if (!pool_ptr) {
        Logger::error("Generation pool is nullptr");
        return;
    }
    switch (m_load_style) {
    case RANDOM:
        // Enqueue directly in random order
        for (auto& task : new_chunks) {
            const ChunkPos POS = task.pos;
            const uint64_t GENERATION_ID = task.generation_id;
            pool_ptr->enqueue([chunk = std::move(task.chunk), POS,
                               GENERATION_ID, this]() mutable {
                chunk->load_or_gen_chunk();
                m_finished_chunks.push(FinishedChunk{
                    .pos = POS,
                    .generation_id = GENERATION_ID,
                    .chunk = std::move(chunk),
                });
            });
        }
        break;
    case CENTER: {
        std::vector<std::pair<ChunkPos, PendingChunk*>> tasks;
        for (auto& task : new_chunks) {

            tasks.emplace_back(task.pos, &task);
        }
        glm::vec3 player_pos = m_world.get_player_pos(uuid);
        ChunkPos player_chunk_pos = get_chunk_pos(player_pos.x, player_pos.z);
        auto dist2 = [player_chunk_pos](ChunkPos chunk_pos) {
            float dx = player_chunk_pos.x - chunk_pos.x;
            float dz = player_chunk_pos.z - chunk_pos.z;
            return dx * dx + dz * dz;
        };

        std::sort(tasks.begin(), tasks.end(),
                  [&dist2](const auto& a, const auto& b) {
                      return dist2(a.first) < dist2(b.first);
                  });

        const int CHUNKS_PER_PRIORITY =
            std::max(1, m_generation_threads.load());

        for (size_t i = 0; i < tasks.size(); ++i) {
            int priority = 10 + static_cast<int>(i / CHUNKS_PER_PRIORITY);
            auto* task = tasks[i].second;
            const ChunkPos POS = task->pos;
            const uint64_t GENERATION_ID = task->generation_id;
            pool_ptr->enqueue(
                priority, [this, POS, GENERATION_ID,
                           chunk = std::move(task->chunk)]() mutable {
                    chunk->load_or_gen_chunk();
                    m_finished_chunks.push(
                        FinishedChunk{.pos = POS,
                                      .generation_id = GENERATION_ID,
                                      .chunk = std::move(chunk)});
                });
        }
    } break;
    }
}

void ServerChunkSystem::start_gen_thread() {
    m_scheduler_alive = false;
    m_generation_running = true;
    Logger::info("Generation Scheduler thread Started");

    m_generation_scheduler = std::jthread([this](std::stop_token token) {
        tracy::SetThreadName("Generation Scheduler");
        m_scheduler_alive = true;
        while (!token.stop_requested()) {
            std::unique_lock<std::mutex> lk(m_generation_queue_mutex);

            m_generation_cv.wait(lk, token, [this]() {
                return m_need_generation.load() || !m_generation_running ||
                       !m_generation_queue.empty();
            });
            if (!m_generation_running) {
                break;
            }
            if (token.stop_requested()) {
                break;
            }
            m_need_generation = false;
            std::optional<Uuid> uuid;
            if (!m_generation_queue.empty()) {
                uuid = m_generation_queue.front();
                m_generation_queue.pop();
            }
            lk.unlock();
            if (uuid) {
                gen_chunks_internal(*uuid);
            }
        }
        m_scheduler_alive = false;
    });
}

void ServerChunkSystem::stop_gen_thread() {
    m_generation_running = false;
    m_generation_cv.notify_all();
    m_generation_scheduler.request_stop();
    if (m_generation_scheduler.joinable()) {
        m_generation_scheduler.join();
    }
    m_scheduler_alive = false;
    Logger::info("Generation Scheduler Thread Stopped");
}

void ServerChunkSystem::stop_generation_pool() {
    auto pool_ptr = m_generation_pool.load();
    if (pool_ptr) {
        pool_ptr->stop();
    }
    m_generation_pool.store(nullptr);
    Logger::info("Generation Pool Stopped");
}

void ServerChunkSystem::request_generation(const Uuid& uuid) {
    if (!m_scheduler_alive) {
        return;
    }
    // if (!m_could_gen) {
    //     Logger::warn("It is generating or consuming new chunks");
    //     return;
    // }

    m_could_generate = false;

    {
        std::lock_guard lock(m_generation_queue_mutex);
        m_generation_queue.enqueue(uuid);
    }

    // m_gen_player_pos = get_player("TestPlayer").get_player_pos();

    m_need_generation = true;

    m_generation_cv.notify_one();
}

void ServerChunkSystem::hot_reload() {
    int dist = m_world.get_config().get("server_distance", 24);
    m_render_distance = std::clamp(dist, 0, MAX_DISTANCE);
}

uint32_t ServerChunkSystem::get_chunk_ref_count(ChunkPos pos) const {
    chunk_cacc cacc;
    if (!m_chunks.find(cacc, pos)) {
        return 0;
    }
    return cacc->second.ref_count;
}

int ServerChunkSystem::get_block(const glm::ivec3& pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(pos.x, pos.z);
    chunk_cacc cacc;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        return 0;
    }
    if (cacc->second.state != ChunkState::READY) {
        return 0;
    }

    return cacc->second.chunk->get_block(pos);
}

bool ServerChunkSystem::is_solid(const glm::ivec3& pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(pos.x, pos.z);
    chunk_cacc cacc;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        return false;
    }
    if (cacc->second.state != ChunkState::READY) {
        return 0;
    }

    auto id = cacc->second.chunk->get_block(pos);
    if (BlockManager::is_gas(id) || BlockManager::is_liquid(id)) {
        return false;
    } else {
        return true;
    }
}

bool ServerChunkSystem::can_pass_block(const glm::ivec3& pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(pos.x, pos.z);
    chunk_cacc cacc;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        return true;
    }
    if (cacc->second.state != ChunkState::READY) {
        return 0;
    }

    auto id = cacc->second.chunk->get_block(pos);
    return BlockManager::is_passable(id);
}

BlockType ServerChunkSystem::get_block_type(const glm::ivec3& pos) const {
    auto [chunk_x, chunk_z] = get_chunk_pos(pos.x, pos.z);
    chunk_cacc cacc;

    if (!m_chunks.find(cacc, ChunkPos{chunk_x, chunk_z})) {
        // Logger::error("Can't Find Block {} {} {}", block_pos.x, block_pos.y,
        //               block_pos.z);
        return 0;
    }
    if (cacc->second.state != ChunkState::READY) {
        return 0;
    }

    return cacc->second.chunk->get_block(pos);
}

bool ServerChunkSystem::set_block(const glm::ivec3& pos, BlockType id) {
    int world_x, world_z;
    world_x = pos.x;
    world_z = pos.z;

    auto [chunk_x, chunk_z] = get_chunk_pos(world_x, world_z);
    chunk_acc acc;

    if (!m_chunks.find(acc, ChunkPos{chunk_x, chunk_z})) {
        return false;
    }
    if (acc->second.state != ChunkState::READY) {
        return false;
    }

    return acc->second.chunk->set_block(pos, id);
}

void ServerChunkSystem::set_render_distance(int distance) {
    m_render_distance = distance;
}

int ServerChunkSystem::render_distance() const { return m_render_distance; }

void ServerChunkSystem::set_load_style(int id) {
    using enum ChunkLoadStyle;

    switch (id) {
    case std::to_underlying(RANDOM):
        m_load_style = RANDOM;
        return;
    case std::to_underlying(CENTER):
        m_load_style = CENTER;
        return;
    }
    Logger::error("Can,t Find Chunk Load Style Id {}, Nothing Will Do", id);
}

bool ServerChunkSystem::is_chunk_active(ChunkPos pos) const {
    chunk_cacc acc;
    if (!m_chunks.find(acc, pos)) {
        return false;
    }

    return acc->second.state == ChunkState::READY &&
           acc->second.ref_count.load(std::memory_order_relaxed) > 0;
}

ChunkLoadStyle ServerChunkSystem::load_style() const { return m_load_style; }

int ServerChunkSystem::generation_threads() const {
    return m_generation_threads;
}
size_t ServerChunkSystem::chunk_size() const { return m_chunks.size(); }
ChunkStorage* ServerChunkSystem::get_storage() { return m_storage.get(); }
} // namespace cubed
