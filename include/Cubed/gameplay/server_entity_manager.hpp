#pragma once
#include "Cubed/gameplay/ecs/entity.hpp"
#include "glm/ext/vector_float3.hpp"

#include <entt/entt.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
namespace Cubed {
class ServerWorld;
class Session;
class ServerEntityManager {
public:
    ServerEntityManager(ServerWorld& world);

    void init();
    void update();
    // not thread safe
    void add_entity(std::string_view name, const glm::vec3& world_pos);
    void destory(EntityID id);
    void handle_player_login(std::shared_ptr<Session> session);

private:
    enum class Command { CREATE, SEND_ALL_ENTITIES, DESTORY };
    struct EntityCreateElement {
        std::string name;
        glm::vec3 pos;
    };
    using EntityMap = tbb::concurrent_hash_map<EntityID, entt::entity>;
    using acc = EntityMap::accessor;
    using cacc = EntityMap::const_accessor;
    using CreateFunc = std::function<EntityID()>;
    using TaskElement =
        std::variant<std::shared_ptr<Session>, EntityCreateElement, EntityID>;
    using TaskPair = std::pair<Command, TaskElement>;
    ServerWorld& m_world;
    tbb::concurrent_queue<TaskPair> m_tasks;
    entt::registry m_registry;
    EntityID m_next = 0;
    EntityMap m_entities;
    std::unordered_map<std::string_view, CreateFunc> m_factories;
    void create_entity(std::string_view name, const glm::vec3& pos);
    void handle_entity_create(EntityID id, std::string_view name,
                              const glm::vec3& pos);
    void handle_entity_destory(EntityID id);
    void handle_task();
    void send_all_entities(std::shared_ptr<Session>& session);
    void update_ai(entt::entity e);
    void update_move(entt::entity e);
    void update_send(entt::entity e,
                     std::span<std::shared_ptr<Session>> sessions);
    template <typename... Args>
    EntityID create_entity_in_factory(Args&&... args) {
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