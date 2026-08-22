#pragma once
#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/ecs/entity.hpp"
#include "Cubed/gameplay/gait.hpp"
#include "Cubed/gameplay/server/entity_storage.hpp"
#include "Cubed/tools/log.hpp"
#include "glm/ext/vector_float3.hpp"

#include <entt/entt.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>
#include <variant>
namespace cubed {
class ServerWorld;
class Session;
class ServerEntityManager {
public:
    static constexpr size_t PER_CREATURE_LIMITS = 100;

    ServerEntityManager(ServerWorld& world);

    void init();

    void update();
    void add_creature(std::string_view name, const glm::vec3& world_pos);
    void add_item_entity(std::string_view name, const glm::vec3& world_pos,
                         const glm::vec3& initial_velocity);
    void destroy(EntityID id);
    void handle_player_login(std::shared_ptr<Session> session);
    void save_all_entities(bool immediately);
    size_t max_creature_sum() const;
    size_t creature_sum() const;
    size_t entity_sum() const;
    EntityID get_next_value() const;
    void set_next_value(EntityID id);
    void unload(EntityID id);
    void stop();

    void add_dormant(EntityStorageData data);
    void activate_chunk(ChunkPos pos);

private:
    enum class Command {
        CREATURE_CREATE,
        SEND_ALL_ENTITIES,
        DESTROY,
        SAVE_ALL,
        UNLOAD,
        ITEM_CREATE
    };
    struct EntityCreateElement {
        std::string name;
        glm::vec3 pos;
    };

    struct ItemEntityCreateElement {
        std::string name;
        glm::vec3 pos;
        glm::vec3 initial_velocity;
    };

    struct EntitySendData {
        EntityID id;
        glm::vec3 pos;
        glm::vec3 dir;
        Gait gait = Gait::STOP;
    };

    using EntityMap = tbb::concurrent_hash_map<EntityID, entt::entity>;
    using acc = EntityMap::accessor;
    using cacc = EntityMap::const_accessor;
    using CreateFunc = std::function<void(EntityID id)>;
    using TaskElement =
        std::variant<std::shared_ptr<Session>, EntityCreateElement,
                     ItemEntityCreateElement, EntityID, std::monostate>;
    using TaskPair = std::pair<Command, TaskElement>;
    using DormantEntityMap =
        std::unordered_map<ChunkPos, std::vector<EntityStorageData>,
                           ChunkPos::Hash>;
    ServerWorld& m_world;
    std::unique_ptr<EntityStorage> m_storage;

    DormantEntityMap m_dormant_entities;

    std::atomic<size_t> m_creature_sum{0};
    std::atomic<size_t> m_entity_sum{0};
    tbb::concurrent_queue<TaskPair> m_tasks;
    entt::registry m_registry;
    EntityID m_next = 0;
    EntityMap m_entities;
    std::unordered_map<std::string, CreateFunc> m_factories;
    void create_entity(const std::string& name, const glm::vec3& pos);
    void create_item_entity(const std::string& name, const glm::vec3& pos,
                            const glm::vec3& velocity);
    void handle_entity_create(EntityID id, std::string_view name,
                              const glm::vec3& pos);
    void handle_entity_destroy(EntityID id);
    bool destroy_internal(EntityID id);
    void handle_task();
    void send_all_entities(std::shared_ptr<Session>& session);
    void update_ai(entt::entity e);
    void update_move(entt::entity e);
    void update_send(entt::entity e,
                     tbb::concurrent_vector<EntitySendData>& sessions);
    void save_all();
    void save(EntityID id);
    void save(entt::entity e);
    void unload_internal(EntityID id);
    std::optional<EntityStorageData> build_entity_storage_data(EntityID id);
    std::optional<EntityStorageData> build_entity_storage_data(entt::entity id);

    void create_item_entity(EntityID id, const std::string& name);

    template <typename... Args>
    void create_entity_in_factory(EntityID id, Args&&... args) {
        auto entity = m_registry.create();

        ((m_registry.emplace<std::remove_cvref_t<Args>>(
             entity, std::forward<Args>(args))),
         ...);
        if (!m_entities.emplace(id, entity)) {
            Logger::error("Can't emplace entity id {}", id);
        } else {
            ++m_entity_sum;
        }

        return;
    }
};
} // namespace cubed
