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
    enum class Command { CREATE };

    ClientEntityManager(ClientWorld& world);
    void update();
    void init();
    // not thread safe
    void add_entity(EntityID id, std::string_view name, const glm::vec3& pos);

    void receive_entity_create(S2CEntityCreate& s2c);

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
    using TaskElement = std::variant<EntityCreateElement>;
    using TaskPair = std::pair<Command, TaskElement>;

    ClientWorld& m_world;
    entt::registry m_registry;
    EntityMap m_entities;
    std::unordered_map<std::string_view, CreateFunc> m_factories;
    tbb::concurrent_queue<TaskPair> m_tasks;

    void handle_task();

    template <typename... Args> void add_entity(EntityID id, Args&&... args) {
        auto entity = m_registry.create();

        ((m_registry.emplace<std::remove_cvref_t<Args>>(
             entity, std::forward<Args>(args))),
         ...);
        m_entities.emplace(id, entity);
        return;
    }
};
} // namespace Cubed