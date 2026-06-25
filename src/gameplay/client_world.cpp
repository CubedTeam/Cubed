#include "Cubed/gameplay/client_world.hpp"

#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/packet.hpp"
using namespace std::chrono;
using namespace std::chrono_literals;
namespace Cubed {

namespace {
struct ChunkRenderData {
    std::array<const std::vector<BlockType>*, 4> neighbor_block;
    ClientChunk* chunk;
};
} // namespace

ClientWorld::ClientWorld(std::string_view player_name,
                         std::shared_ptr<NetworkClient> client)
    : m_player(*this, player_name), m_client(client) {}

ClientWorld::~ClientWorld() {
    stop_client_thread();

    {
        std::lock_guard lock(m_chunks_mutex);
        m_chunks.clear();
    }
    {
        std::lock_guard lk(m_delete_vbo_mutex);
        for (auto x : m_pending_delete_vbo) {
            glDeleteBuffers(1, &x);
        }
        m_pending_delete_vbo.clear();
    }
    {
        std::lock_guard lk(m_delete_vao_mutex);
        for (auto x : m_pending_delete_vao) {
            glDeleteVertexArrays(1, &x);
        }
        m_pending_delete_vao.clear();
    }
    m_timers.clear();
}

const std::optional<LookBlock>&
ClientWorld::get_look_block_pos(const std::string& name) const {

    return m_player.get_look_block_pos();
}

ClientPlayer& ClientWorld::get_player() { return m_player; }

void ClientWorld::init() {
    m_chunks.reserve(MAX_DISTANCE * MAX_DISTANCE * 4);

    // timer
    register_timer("player_pos", 2, [this]() { report_player_pos(); });

    LoginReq req;
    req.set_name(m_player.get_name());
    // request login
    m_client->send(make_packet(req));
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
}

void ClientWorld::stop_client_thread() {
    m_client_thread.request_stop();
    if (m_client_thread.joinable()) {
        m_client_thread.join();
    }
    m_game_running = false;
}

void ClientWorld::client_run(std::stop_token stoken) {
    Logger::info("Client Thread Started");
    while (!stoken.stop_requested()) {
        for (auto& x : m_timers) {
            x.second.update();
        }
        std::this_thread::sleep_for(milliseconds(DEFAULT_PER_TICK_TIME));
    }
}

void ClientWorld::report_player_pos() {
    if (!m_client) {
        return;
    }
    PlayerPos pos;
    pos.set_uuid(m_player.get_uuid());
    glm::vec3 player_pos = m_player.get_player_pos();
    auto* v3 = pos.mutable_pos();
    v3->set_x(player_pos.x);
    v3->set_y(player_pos.y);
    v3->set_z(player_pos.z);
    m_client->send(make_packet(pos));
}

void ClientWorld::request_chunk() {
    ChunkPosSet required_chunks;

    glm::vec3 player_pos = m_player.get_player_pos();

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

    ChunkPosSet need_send_pos;
    {
        std::lock_guard lk(m_chunks_mutex);
        for (auto it = m_chunks.begin(); it != m_chunks.end();) {
            if (required_chunks.find(it->first) == required_chunks.end()) {
                it = m_chunks.unsafe_erase(it);
            } else {
                ++it;
            }
        }

        for (auto pos : required_chunks) {
            auto it = m_chunks.find(pos);
            if (it == m_chunks.end()) {
                need_send_pos.emplace(pos);
            }
        }
    }
    if (need_send_pos.empty()) {
        return;
    }
    auto uuid = m_player.get_uuid();
    ChunkDataReq req;
    for (const auto& pos : need_send_pos) {
        req.set_uuid(uuid);
        auto* p = req.mutable_pos();
        p->set_x(pos.x);
        p->set_z(pos.z);
        m_client->send(make_packet(req));
    }
}

void ClientWorld::receive_chunk(const ChunkDataRsp& data) {
    ClientChunk chunk{*this};
    chunk.receive_chunk(data);
    {
        std::lock_guard lock(m_pending_queue_mutex);
        m_pending_queue.emplace_back(std::move(chunk));
    }
}

void ClientWorld::update(float delta_time) {
    m_player.update(delta_time);
    {
        std::lock_guard lk(m_delete_vbo_mutex);
        for (auto x : m_pending_delete_vbo) {
            glDeleteBuffers(1, &x);
        }
        m_pending_delete_vbo.clear();
    }

    {
        std::lock_guard lk(m_delete_vao_mutex);
        for (auto x : m_pending_delete_vao) {
            glDeleteVertexArrays(1, &x);
        }
        m_pending_delete_vao.clear();
    }
    std::vector<ClientChunk> new_chunks;
    {
        std::lock_guard lock(m_pending_queue_mutex);
        for (auto& c : m_pending_queue) {
            new_chunks.emplace_back(std::move(c));
        }
    }
    for (auto& c : new_chunks) {
        c.upload_to_gpu();
    }
    {
        std::lock_guard lock(m_chunks_mutex);
        for (auto& c : new_chunks) {
            m_chunks.emplace(c.get_chunk_pos(), std::move(c));
        }
        m_render_snapshots.clear();
        for (auto& [pos, chunk] : m_chunks) {
            if (chunk.is_dirty()) {
                // the curial fator influence
                OptionalBlockVectorArray neighbor_block;
                for (int i = 0; i < 4; i++) {
                    auto it = m_chunks.find(pos + CHUNK_DIR[i]);
                    if (it != m_chunks.end()) {
                        neighbor_block[i] = (it->second.get_chunk_blocks());
                    } else {
                        neighbor_block[i] = std::nullopt;
                    }
                }
                chunk.gen_vertex_data(neighbor_block);
                chunk.upload_to_gpu();
            }
            if (!chunk.is_dirty()) {
                if (chunk.is_need_upload()) {
                    chunk.upload_to_gpu();
                }
                m_render_snapshots.push_back(
                    {chunk.get_normal_vao(), chunk.get_normal_vertices_sum(),
                     chunk.get_cross_vao(), chunk.get_cross_vertices_sum(),
                     chunk.get_normal_discard_vao(),
                     chunk.get_normal_discard_vertices_sum(),
                     chunk.get_normal_blend_vao(),
                     chunk.get_normal_blend_vertices_sum(),
                     chunk.get_water_vao(), chunk.get_water_vertices_sum(),
                     glm::vec3(static_cast<float>(pos.x * CHUNK_SIZE) +
                                   static_cast<float>(CHUNK_SIZE / 2),
                               static_cast<float>(WORLD_SIZE_Y / 2),
                               static_cast<float>(pos.z * CHUNK_SIZE) +
                                   static_cast<float>(CHUNK_SIZE / 2)),
                     glm::vec3(static_cast<float>(CHUNK_SIZE / 2),
                               static_cast<float>(WORLD_SIZE_Y / 2),
                               static_cast<float>(CHUNK_SIZE / 2))});
            }
        }
    }
}

} // namespace Cubed