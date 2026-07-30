#pragma once
#include "Cubed/gameplay/ecs/entity.hpp"
#include "glm/ext/vector_float3.hpp"
#include "world/entity.pb.h"

#include <entt/entt.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
namespace Cubed {
class ClientWorld;
class ClientEntityManager {
public:
    enum class Command { CREATE, DESTORY };

    ClientEntityManager(ClientWorld& world);
    void update();
    void init();

    void receive_entity_create(S2CEntityCreate& s2c);
    void receive_entity_destory(EntityID id);

    void destory(EntityID id);
    void create(std::string_view name, const glm::vec3& pos);

    const entt::registry& get_registry() const;

private:
    struct EntityCreateElement {
        EntityID id;
        std::string name;
        glm::vec3 pos;
    };
    using EntityMap = tbb::concurrent_hash_map<EntityID, entt::entity>;
    using acc = EntityMap::accessor;
    using cacc = EntityMap::const_accessor;
    using CreateFunc = std::function<void(EntityID id)>;
    using TaskElement = std::variant<EntityCreateElement, EntityID>;
    using TaskPair = std::pair<Command, TaskElement>;

    ClientWorld& m_world;
    entt::registry m_registry;
    EntityMap m_entities;
    std::unordered_map<std::string_view, CreateFunc> m_factories;
    tbb::concurrent_queue<TaskPair> m_tasks;

    void handle_task();
    void handle_entity_destory(EntityID id);
    // not thread safe
    void handle_entity_create(EntityID id, std::string_view name,
                              const glm::vec3& pos);
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
} // namespace Cubed