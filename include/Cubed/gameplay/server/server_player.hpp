#pragma once
#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/gait.hpp"
#include "Cubed/gameplay/game_mode.hpp"
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/item_stack.hpp"
#include "Cubed/gameplay/server/server_chunk.hpp"
#include "Cubed/tools/uuid.hpp"
#include "player/inventory.pb.h"

#include <array>
#include <atomic>
#include <glm/glm.hpp>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <tbb/concurrent_queue.h>
namespace cubed {
class ServerWorld;
class Session;
class ServerPlayer {

public:
    struct MoveAction {
        uint64_t revision = 0;
        uint64_t request_id = 0;
        size_t from = 0;
        size_t to = 0;
    };
    struct AddAction {
        uint64_t revision = 0;
        uint64_t request_id = 0;
        size_t position = 0;
        ItemID item = 0;
        size_t count = 0;
    };
    struct RemoveAction {
        uint64_t revision = 0;
        uint64_t request_id = 0;
        size_t position = 0;
        size_t count = 0;
    };
    using Inventory = std::array<std::optional<ItemStack>, INVENTORY_SIZE>;
    enum class Task { ADD_ITEM, REMOVE_ITEM, SEND_ALL_INVENTORY, MOVE_ITEM };
    ServerPlayer(const ServerPlayer&) = delete;
    ServerPlayer(ServerPlayer&&) = delete;
    ServerPlayer& operator=(const ServerPlayer&) = delete;
    ServerPlayer& operator=(ServerPlayer&&) = delete;
    ServerPlayer(std::string_view name, Uuid uuid, ServerWorld& m_world,
                 std::shared_ptr<Session> session, TickType gametick);

    void update();

    glm::vec3 get_pos() const;
    const std::string& get_name() const;

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

    void add(AddAction action);
    void send_all_inventory();
    void init_add(ItemStack item, size_t position);
    void unsafe_add(AddAction action);
    // Return the number of successfully added items
    uint32_t atomic_add_item(ItemID id, uint32_t count);
    void remove(RemoveAction action);
    void move(MoveAction action);
    void handle_inventory_action(protocol::C2SInventoryAction& msg);

    Inventory inventory_snapshot() const;

private:
    using TaskElement =
        std::variant<AddAction, RemoveAction, MoveAction, std::monostate>;
    using TaskPair = std::pair<Task, TaskElement>;
    static constexpr TickType TIMEOUT = 200;
    const std::string M_NAME;
    const Uuid M_UUID;
    std::atomic<GameMode> m_mode{GameMode::CREATIVE};
    ServerWorld& m_world;

    mutable std::shared_mutex m_inventory_mutex;
    Inventory m_inventory;
    uint64_t m_revision = 1;

    tbb::concurrent_queue<TaskPair> m_task;
    std::atomic<glm::vec3> m_pos{glm::vec3{0.0f, 255.0f, 0.0f}};
    ChunkPos m_last_chunk_pos{0, 0};

    std::atomic<std::shared_ptr<Session>> m_session;
    std::atomic<TickType> m_last_gametick{0};
    std::atomic<int> m_chunk_task_id{0};
    std::atomic<float> m_yaw{0.0f};
    std::atomic<float> m_pitch{0.0f};
    std::atomic<Gait> m_gait{Gait::STOP};
    mutable std::shared_mutex m_chunk_pos_mutex;
    ChunkPosSet m_player_chunk_pos_set;

    void add_internal(const AddAction& action);
    void remove_internal(const RemoveAction& action);
    void move_internal(const MoveAction& action);
    void send_all_inventory_internal(uint64_t request_id = 0);
};
} // namespace cubed
