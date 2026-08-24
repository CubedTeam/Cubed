#pragma once
#include "Cubed/gameplay/ecs/entity.hpp"
#include "Cubed/gameplay/gait.hpp"
#include "Cubed/gameplay/model.hpp"
#include "Cubed/tools/cubed_random.hpp"
#include "glm/ext/vector_float3.hpp"
#include "world/entity.pb.h"

#include <entt/entt.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
namespace cubed {
class ClientWorld;
class ClientEntityManager {
public:
    enum class Command { CREATE, DESTROY, UPDATE };

    ClientEntityManager(ClientWorld& world);
    void update(float dt);
    void init();

    void receive_entity_create(protocol::S2CEntityCreate& msg);
    void receive_entity_destroy(EntityID id);
    void receive_entity_update(const protocol::S2CEntityUpdate& msg);
    void receive_entity_update(protocol::S2CEntityUpdateBatch& msg);
    void destroy(EntityID id);
    void create(std::string_view name, const glm::vec3& pos);

    const entt::registry& get_registry() const;

    void player_sound(float dt);

private:
    struct EntityCreateElement {
        EntityID id;
        std::string name;
        glm::vec3 pos;
    };

    struct UpdateInfo {
        EntityID id;
        glm::vec3 pos;
        glm::vec3 direction;
        Gait gait;
    };

    using EntityMap = tbb::concurrent_hash_map<EntityID, entt::entity>;
    using acc = EntityMap::accessor;
    using cacc = EntityMap::const_accessor;
    using CreateFunc = std::function<void(EntityID id)>;
    using TaskElement = std::variant<EntityCreateElement, EntityID, UpdateInfo>;
    using TaskPair = std::pair<Command, TaskElement>;

    ClientWorld& m_world;
    entt::registry m_registry;
    EntityMap m_entities;
    std::unordered_map<std::string, CreateFunc> m_factories;
    tbb::concurrent_queue<TaskPair> m_tasks;
    Random m_random;
    void handle_task(float dt);
    void handle_entity_destroy(EntityID id);
    // not thread safe
    void handle_entity_create(EntityID id, const std::string& name,
                              const glm::vec3& pos);
    void handle_entity_update(UpdateInfo& info, float dt);
    void create_item_entity(EntityID id, ModelID model, std::string_view name);
    template <typename... Args>
    void create_entity_in_registry(EntityID id, Args&&... args) {
        {
            cacc a;
            if (m_entities.find(a, id)) {
                return;
            }
        }
        auto entity = m_registry.create();

        ((m_registry.emplace<std::remove_cvref_t<Args>>(
             entity, std::forward<Args>(args))),
         ...);
        m_entities.emplace(id, entity);
        return;
    }
};
} // namespace cubed
