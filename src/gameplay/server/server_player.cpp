#include "Cubed/gameplay/server/server_player.hpp"

#include "Cubed/gameplay/server/server_world.hpp"
namespace cubed {
ServerPlayer::ServerPlayer(std::string_view name, Uuid uuid, ServerWorld& world,
                           std::shared_ptr<Session> session, TickType gametick)
    : M_NAME(name), M_UUID(std::move(uuid)), m_world(world), m_session(session),
      m_last_gametick(gametick) {}

void ServerPlayer::update() {
    TaskPair task;
    while (m_task.try_pop(task)) {
        switch (task.first) {
        case Task::ADD_ITEM: {
            auto* v = std::get_if<ItemStackPair>(&task.second);
            ASSERT(v);
            add_internal(std::move(v->second), v->first);
        } break;
        case Task::REMOVE_ITEM: {
            auto* v = std::get_if<size_t>(&task.second);
            ASSERT(v);
            remove_internal(*v);
        } break;
        }
    }
}

glm::vec3 ServerPlayer::get_pos() const { return m_pos.load(); }
const std::string& ServerPlayer::get_name() const { return M_NAME; }

std::shared_ptr<Session> ServerPlayer::get_session() const { return m_session; }
void ServerPlayer::update_pos(float x, float y, float z) {
    m_pos = glm::vec3{x, y, z};
    ChunkPos chunk_pos = get_chunk_pos(x, z);
    float dist = distance2(chunk_pos, m_last_chunk_pos);
    if (dist > 2) {
        m_world.request_generation(M_UUID);
        m_last_chunk_pos = chunk_pos;
    }
}

void ServerPlayer::update_sync_gametick(TickType gametick) {
    m_last_gametick = gametick;
}
bool ServerPlayer::is_disconnect(TickType current_gametick) const {
    if (current_gametick - m_last_gametick > TIMEOUT) {
        return true;
    }
    return false;
}

int ServerPlayer::task_id() const { return m_chunk_task_id.load(); }

bool ServerPlayer::has_player(ChunkPos pos) const {
    std::shared_lock lock(m_chunk_pos_mutex);
    return m_player_chunk_pos_set.find(pos) != m_player_chunk_pos_set.end();
}
void ServerPlayer::update_chunk_set(const ChunkPosSet& set) {
    std::lock_guard lock(m_chunk_pos_mutex);
    m_player_chunk_pos_set.clear();
    m_player_chunk_pos_set.insert(set.begin(), set.end());
}

ChunkPosSet ServerPlayer::get_chunk_pos_set() const {
    std::shared_lock lock(m_chunk_pos_mutex);
    return m_player_chunk_pos_set;
}

ChunkPosSet ServerPlayer::take_chunk_pos_set() {
    std::lock_guard lock(m_chunk_pos_mutex);
    return std::exchange(m_player_chunk_pos_set, {});
}

void ServerPlayer::update_task_id_max(int new_id) {
    int current = m_chunk_task_id.load(std::memory_order_relaxed);
    while (current < new_id && !m_chunk_task_id.compare_exchange_weak(
                                   current, new_id, std::memory_order_relaxed,
                                   std::memory_order_relaxed)) {
    }
}

void ServerPlayer::add(ItemStack item, size_t position) {
    m_task.emplace(Task::ADD_ITEM, ItemStackPair{position, std::move(item)});
}

void ServerPlayer::remove(size_t position) {
    m_task.emplace(Task::REMOVE_ITEM, position);
}

void ServerPlayer::set_yaw(float yaw) { m_yaw = yaw; }
void ServerPlayer::set_pitch(float pitch) { m_pitch = pitch; }
float ServerPlayer::yaw() const { return m_yaw.load(); }
float ServerPlayer::pitch() const { return m_pitch.load(); }
Gait ServerPlayer::gait() const { return m_gait; }
void ServerPlayer::set_gait(Gait gait) { m_gait = gait; }
Uuid ServerPlayer::get_uuid() const { return M_UUID; }

void ServerPlayer::add_internal(ItemStack item, size_t position) {
    m_inventory[position] = std::move(item);
}
void ServerPlayer::remove_internal(size_t position) {
    m_inventory[position] = std::nullopt;
}

} // namespace cubed
