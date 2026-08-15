#pragma once
#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/gait.hpp"
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/server/server_chunk.hpp"
#include "Cubed/tools/uuid.hpp"

#include <atomic>
#include <glm/glm.hpp>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
namespace Cubed {
class ServerWorld;
class Session;
class ServerPlayer {

public:
    ServerPlayer(const ServerPlayer&) = delete;
    ServerPlayer(ServerPlayer&&) = delete;
    ServerPlayer& operator=(const ServerPlayer&) = delete;
    ServerPlayer& operator=(ServerPlayer&&) = delete;
    ServerPlayer(std::string_view name, Uuid uuid, ServerWorld& m_world,
                 std::shared_ptr<Session> session, TickType gametick);

    glm::vec3 get_pos() const;
    const std::string& get_name() const;
    std::string get_uuid_string() const;
    std::shared_ptr<Session> get_session() const;

    void update_sync_gametick(TickType gametick);
    bool is_disconnect(TickType current_gametick) const;
    int task_id() const;
    bool has_player(ChunkPos pos) const;
    void update_chunk_set(const ChunkPosSet& set);
    ChunkPosSet get_chunk_pos_set() const;
    ChunkPosSet take_chunk_pos_set();
    void set_yaw(float yaw);
    void set_pitch(float pitch);
    float yaw() const;
    float pitch() const;

    Gait gait() const;
    void set_gait(Gait gait);

    void update_task_id_max(int new_id);
    void update_pos(float x, float y, float z);
    Uuid get_uuid() const;

private:
    static constexpr TickType TIMEOUT = 200;
    const std::string M_NAME;
    const Uuid M_UUID;
    std::atomic<glm::vec3> m_pos{glm::vec3(0.0f)};
    ServerWorld& m_world;
    ChunkPos m_last_chunk_pos{0, 0};
    std::atomic<std::shared_ptr<Session>> m_session;
    std::atomic<TickType> m_last_gametick{0};
    std::atomic<int> m_chunk_task_id{0};
    std::atomic<float> m_yaw{0.0f};
    std::atomic<float> m_pitch{0.0f};
    std::atomic<Gait> m_gait{Gait::STOP};
    mutable std::shared_mutex m_chunk_pos_mutex;
    ChunkPosSet m_player_chunk_pos_set;
};
} // namespace Cubed
