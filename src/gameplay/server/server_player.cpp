#include "Cubed/gameplay/server/server_player.hpp"

#include "Cubed/gameplay/packet.hpp"
#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/gameplay/server/session.hpp"
using namespace google::protobuf;
namespace cubed {
ServerPlayer::ServerPlayer(std::string_view name, Uuid uuid, ServerWorld& world,
                           std::shared_ptr<Session> session, TickType gametick)
    : M_NAME(name), M_UUID(std::move(uuid)), m_world(world), m_session(session),
      m_last_gametick(gametick) {}

void ServerPlayer::update() {
    std::lock_guard lock(m_inventory_mutex);
    TaskPair task;
    while (m_task.try_pop(task)) {
        switch (task.first) {
        case Task::ADD_ITEM: {
            auto* v = std::get_if<AddAction>(&task.second);
            ASSERT(v);
            add_internal(*v);
        } break;
        case Task::REMOVE_ITEM: {
            auto* v = std::get_if<RemoveAction>(&task.second);
            ASSERT(v);
            remove_internal(*v);
        } break;
        case Task::SEND_ALL_INVENTORY: {
            send_all_inventory_internal();
        } break;
        case Task::MOVE_ITEM: {
            auto* v = std::get_if<MoveAction>(&task.second);
            ASSERT(v);
            move_internal(*v);
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

void ServerPlayer::send_all_inventory_internal(uint64_t request_id) {
    Arena arena;
    auto* msg = Arena::Create<protocol::S2CInventoryUpdate>(&arena);

    msg->set_accepted(true);
    msg->set_full_snapshot(true);
    msg->set_request_id(request_id);
    msg->set_revision(m_revision);
    auto* slots = msg->mutable_slots();

    for (size_t i = 0; i < m_inventory.size(); ++i) {
        auto* slot = slots->Add();
        slot->set_position(i);
        auto& stack = m_inventory[i];
        if (stack) {
            auto* s = slot->mutable_stack();
            s->set_count(stack->count);
            s->set_item(stack->item);

        } else {
            slot->set_empty(true);
        }
    }

    get_session()->send(make_packet(msg));
}

void ServerPlayer::add(AddAction action) {
    m_task.emplace(Task::ADD_ITEM, std::move(action));
}
void ServerPlayer::send_all_inventory() {
    m_task.emplace(Task::SEND_ALL_INVENTORY, std::monostate{});
}

void ServerPlayer::init_add(ItemStack item, size_t position) {
    if (position >= INVENTORY_SIZE) {
        ASSERT(false);
        return;
    }
    m_inventory[position] = std::move(item);
}

void ServerPlayer::unsafe_add(AddAction action) { add_internal(action); }

void ServerPlayer::remove(RemoveAction action) {
    m_task.emplace(Task::REMOVE_ITEM, std::move(action));
}
void ServerPlayer::move(MoveAction action) {
    m_task.emplace(Task::MOVE_ITEM, std::move(action));
}
void ServerPlayer::handle_inventory_action(protocol::C2SInventoryAction& msg) {

    if (msg.has_add()) {
        if (m_mode != GameMode::CREATIVE) {
            return;
        }
        AddAction action;
        action.revision = msg.base_revision();
        action.stack.count = msg.add().count();
        action.stack.item = msg.add().item();
        action.request_id = msg.request_id();
        if (msg.add().to() >= INVENTORY_SIZE) {
            return;
        }
        action.position = msg.add().to();
        add(std::move(action));
    }

    if (msg.has_remove()) {
        if (msg.remove().from() >= INVENTORY_SIZE) {
            return;
        }
        RemoveAction action;
        action.position = msg.remove().from();
        action.revision = msg.base_revision();
        action.request_id = msg.request_id();
        remove(std::move(action));
    }

    if (msg.has_move()) {
        MoveAction action;

        action.from = msg.move().from();
        action.to = msg.move().to();
        action.revision = msg.base_revision();
        action.request_id = msg.request_id();

        if (action.from >= INVENTORY_SIZE || action.to >= INVENTORY_SIZE) {
            return;
        }

        move(std::move(action));
    }
}

ServerPlayer::Inventory ServerPlayer::inventory_snapshot() const {
    std::shared_lock lock(m_inventory_mutex);
    return m_inventory;
}
void ServerPlayer::set_yaw(float yaw) { m_yaw = yaw; }
void ServerPlayer::set_pitch(float pitch) { m_pitch = pitch; }
float ServerPlayer::yaw() const { return m_yaw.load(); }
float ServerPlayer::pitch() const { return m_pitch.load(); }
Gait ServerPlayer::gait() const { return m_gait; }
void ServerPlayer::set_gait(Gait gait) { m_gait = gait; }
Uuid ServerPlayer::get_uuid() const { return M_UUID; }

void ServerPlayer::add_internal(const AddAction& action) {
    if (action.revision != m_revision) {
        send_all_inventory_internal(action.request_id);
        return;
    }
    if (action.position >= INVENTORY_SIZE) {
        ASSERT(false);
        return;
    }

    m_inventory[action.position] = std::move(action.stack);
    ++m_revision;
    send_all_inventory_internal(action.request_id);
}
void ServerPlayer::remove_internal(const RemoveAction& action) {
    if (action.revision != m_revision) {
        send_all_inventory_internal(action.request_id);
        return;
    }
    if (action.position >= INVENTORY_SIZE) {
        ASSERT(false);
        return;
    }
    m_inventory[action.position] = std::nullopt;
    ++m_revision;
    send_all_inventory_internal(action.request_id);
}

void ServerPlayer::move_internal(const MoveAction& action) {
    if (action.revision != m_revision) {
        send_all_inventory_internal(action.request_id);
        return;
    }
    if (action.from >= INVENTORY_SIZE || action.to >= INVENTORY_SIZE) {
        ASSERT(false);
        return;
    }
    std::swap(m_inventory[action.from], m_inventory[action.to]);
    ++m_revision;
    send_all_inventory_internal(action.request_id);
}

} // namespace cubed
