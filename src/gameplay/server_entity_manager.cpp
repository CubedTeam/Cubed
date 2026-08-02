#include "Cubed/gameplay/server_entity_manager.hpp"

#include "Cubed/gameplay/creatures/pig.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/server_entity.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/gameplay/server_world.hpp"
#include "Cubed/gameplay/session.hpp"
#include "Cubed/gameplay/systems/physical_system.hpp"
#include "Cubed/gameplay/systems/speed_system.hpp"
#include "Cubed/gameplay/systems/wander_ai_system.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/net_utils.hpp"
using namespace google::protobuf;

namespace Cubed {
ServerEntityManager::ServerEntityManager(ServerWorld& world) : m_world(world) {}

void ServerEntityManager::init() {
    m_factories.try_emplace("cubed:pig", [this]() {
        BaseServerCreature c;
        c.hitbox = HitboxManager::instance().get_hitbox_id("cubed:pig");
        c.gravity.value = 1.0f;
        c.movement.acceleration = 1.0f;
        c.movement.deceleration = 1.0f;
        return create_entity_in_factory(
            Entity{m_next}, EntityInfo{"cubed:pig", ""}, std::move(c), PigTag{},
            AIBase{}, WanderAITag{}, MoveBoost{});
    });
}
void ServerEntityManager::update() {
    handle_task();
    update_ai();
    update_move();

    update_send();
}

void ServerEntityManager::update_ai() { WanderAISystem::update(m_registry); }

void ServerEntityManager::update_move() {
    SpeedSystem::update(
        static_cast<float>(m_world.get_per_tick_time()) / 1000.0f, m_registry);
    PhysicalSystem::update(m_world, m_registry);
}

void ServerEntityManager::update_send() {
    auto sessions = m_world.get_all_session();
    auto view = m_registry.view<Entity, BaseServerCreature>();

    for (auto& e : view) {
        auto [entity, creature] = view.get<Entity, BaseServerCreature>(e);
        Arena arena;
        auto* p = Arena::Create<S2CEntityUpdate>(&arena);
        p->set_id(entity.id);
        Tools::set_net_pos(p, creature.transform.position.value);

        for (auto& s : sessions) {
            s->send(make_packet(p));
        }
    }
}

void ServerEntityManager::handle_task() {
    TaskPair pair;
    while (m_tasks.try_pop(pair)) {
        switch (pair.first) {
        case Command::CREATE: {
            auto* c = std::get_if<EntityCreateElement>(&pair.second);
            ASSERT(c);
            create_entity(c->name, c->pos);
        } break;
        case Command::SEND_ALL_ENTITIES: {
            auto* c = std::get_if<std::shared_ptr<Session>>(&pair.second);
            ASSERT(c);
            send_all_entities(*c);
        } break;
        case Command::DESTORY: {
            auto* c = std::get_if<EntityID>(&pair.second);
            ASSERT(c);
            handle_entity_destory(*c);
        } break;
        }
    }
}

void ServerEntityManager::add_entity(std::string_view name,
                                     const glm::vec3& pos) {
    m_tasks.emplace(Command::CREATE,
                    EntityCreateElement{std::string(name), pos});
}

void ServerEntityManager::destory(EntityID id) {
    m_tasks.emplace(Command::DESTORY, id);
}

void ServerEntityManager::handle_player_login(
    std::shared_ptr<Session> session) {
    m_tasks.emplace(Command::SEND_ALL_ENTITIES, std::move(session));
}

void ServerEntityManager::create_entity(std::string_view name,
                                        const glm::vec3& pos) {
    ASSERT(m_factories.contains(name));
    auto e = m_factories[name]();
    acc c;
    if (m_entities.find(c, e)) {
        auto t = m_registry.try_get<BaseServerCreature>(c->second);
        ASSERT(t);
        t->transform.position.value = pos;
    }
    handle_entity_create(e, name, pos);
}

void ServerEntityManager::send_all_entities(std::shared_ptr<Session>& session) {
    auto view = m_registry.view<Entity, EntityInfo, BaseServerCreature>();
    for (auto& entity : view) {
        auto [e, info, base] =
            view.get<Entity, EntityInfo, BaseServerCreature>(entity);
        Arena arena;
        auto* s2c = Arena::Create<S2CEntityCreate>(&arena);
        s2c->set_id(e.id);
        s2c->set_name(info.name);
        Tools::set_net_pos(s2c, base.transform.position.value);
        session->send(make_packet(*s2c));
    }
}

void ServerEntityManager::handle_entity_create(EntityID id,
                                               std::string_view name,
                                               const glm::vec3& pos) {
    auto sessions = m_world.get_all_session();

    Arena arena;
    auto* s2c = Arena::Create<S2CEntityCreate>(&arena);
    s2c->set_id(id);
    s2c->set_name(name);
    Tools::set_net_pos(s2c, pos);

    for (auto& s : sessions) {
        s->send(make_packet(*s2c));
    }
}

void ServerEntityManager::handle_entity_destory(EntityID id) {
    acc a;
    if (!m_entities.find(a, id)) {
        return;
    }
    m_registry.destroy(a->second);
    m_entities.erase(a);
    auto sessions = m_world.get_all_session();
    Arena arena;
    auto* s2c = Arena::Create<S2CEntityDestory>(&arena);
    s2c->set_id(id);
    for (auto& s : sessions) {
        s->send(make_packet(*s2c));
    }
}

} // namespace Cubed