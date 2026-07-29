#pragma once
#include "Cubed/gameplay/ecs/entity.hpp"
#include "glm/ext/vector_float3.hpp"

#include <entt/entt.hpp>
#include <tbb/concurrent_hash_map.h>
namespace Cubed {
class ServerWorld;
class ServerEntityManager {
public:
    ServerEntityManager(ServerWorld& world);

    void init();
    // not thread safe
    void add_entity(std::string_view name, const glm::vec3& pos);

private:
    using EntityMap = tbb::concurrent_hash_map<EntityID, entt::entity>;
    using acc = EntityMap::accessor;
    using cacc = EntityMap::const_accessor;
    using CreateFunc = std::function<EntityID()>;
    ServerWorld& m_world;
    entt::registry m_registry;
    EntityID m_next = 0;
    EntityMap m_entities;
    std::unordered_map<std::string_view, CreateFunc> m_factories;

    void send_entity_create(EntityID id, std::string_view name,
                            const glm::vec3& pos);

    template <typename... Args> EntityID add_entity(Args&&... args) {
        auto entity = m_registry.create();

        ((m_registry.emplace<std::remove_cvref_t<Args>>(
             entity, std::forward<Args>(args))),
         ...);
        auto id = m_next++;
        m_entities.emplace(id, entity);
        return id;
    }
};
} // namespace Cubed