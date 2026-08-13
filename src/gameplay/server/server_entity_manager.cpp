#include "Cubed/gameplay/server/server_entity_manager.hpp"

#include "Cubed/gameplay/creatures/pig.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/server_entity.hpp"
#include "Cubed/gameplay/gait.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/gameplay/server/session.hpp"
#include "Cubed/gameplay/systems/physical_system.hpp"
#include "Cubed/gameplay/systems/speed_system.hpp"
#include "Cubed/gameplay/systems/wander_ai_system.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/proto_utils.hpp"

#include <tracy/Tracy.hpp>
using namespace google::protobuf;

namespace Cubed {
ServerEntityManager::ServerEntityManager(ServerWorld& world) : m_world(world) {}

void ServerEntityManager::init() {
    m_factories.try_emplace("cubed:pig", [this]() {
        BaseServerCreature c;
        c.hitbox = HitboxManager::instance().get_hitbox_id("cubed:pig");
        c.gravity.value = PigDefaults::GRAVITY;
        c.movement.acceleration = PigDefaults::ACCELERATION;
        c.movement.deceleration = PigDefaults::DECELERATION;
        c.velocity.max.x = c.velocity.max.z = PigDefaults::MAX_SPEED;
        return create_entity_in_factory(
            Entity{m_next, EntityType::CREATURE}, EntityInfo{"cubed:pig", ""},
            std::move(c), PigTag{}, AIBase{}, WanderAITag{}, MoveBoost{});
    });
}
void ServerEntityManager::update() {
    ZoneScopedN("Server Entity update");
    handle_task();

    auto view = m_registry.view<BaseServerCreature>();

    auto pool = m_world.get_compute_pool();
    if (!pool) {
        return;
    }
    auto sessions = m_world.get_all_session();
    std::vector<entt::entity> entities;
    for (auto e : view) {
        entities.push_back(e);
    }
    tbb::concurrent_vector<EntitySendData> send_data;
    // parallel block touches disjoint entities only;
    // structural registry changes stay on the server thread via m_tasks.
    parallel_do(
        *pool, entities.begin(), entities.end(), pool->thread_sum(),
        [this, &send_data](entt::entity e) {
            const auto& c = m_registry.get<BaseServerCreature>(e);
            if (!m_world.get_chunk_ref_count(c.transform.position.value)) {
                const auto& entity = m_registry.get<Entity>(e);
                destory(entity.id);
                return;
            }
            update_ai(e);
            update_move(e);
            update_send(e, send_data);
        });
    if (!send_data.empty()) {
        Arena arena;
        auto* msg = Arena::Create<S2CEntityUpdateBatch>(&arena);
        for (auto& data : send_data) {
            auto* u = msg->add_updates();
            u->set_id(data.id);
            Tools::set_proto_pos(u, data.pos);
            Tools::set_proto_vec3(u->mutable_direction(), data.dir);
            u->set_gait(get_gait_id(data.gait));
        }
        auto packet = make_packet(msg);
        for (auto& player : sessions) {
            player->send(packet);
        }
    }
}

void ServerEntityManager::update_ai(entt::entity e) {
    WanderAISystem::update(m_registry, e);
}

void ServerEntityManager::update_move(entt::entity e) {
    SpeedSystem::update(static_cast<float>(m_world.get_per_tick_time()) /
                            1000.0f,
                        m_registry, e);
    PhysicalSystem::update(m_world, m_registry, e);
}

void ServerEntityManager::update_send(
    entt::entity e, tbb::concurrent_vector<EntitySendData>& send_data) {

    if (!m_registry.all_of<Entity, BaseServerCreature>(e)) {
        return;
    }

    const auto [entity, creature] =
        m_registry.get<Entity, BaseServerCreature>(e);
    EntitySendData data;
    data.id = entity.id;
    data.pos = creature.transform.position.value;
    data.dir = creature.transform.direction.value;
    const auto& v = creature.velocity.value;
    if (v.x * v.x + v.z * v.z > 1e-4f) {
        data.gait = Gait::WALK;
    } else {
        data.gait = Gait::STOP;
    }
    send_data.emplace_back(std::move(data));
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

void ServerEntityManager::add_creature(std::string_view name,
                                       const glm::vec3& world_pos) {
    if (m_creature_sum.fetch_add(1) >= max_creature_sum()) {
        m_creature_sum.fetch_sub(1);
        return;
    }
    ++m_entity_sum;
    m_tasks.emplace(Command::CREATE,
                    EntityCreateElement{std::string(name), world_pos});
}

void ServerEntityManager::destory(EntityID id) {
    m_tasks.emplace(Command::DESTORY, id);
}

void ServerEntityManager::handle_player_login(
    std::shared_ptr<Session> session) {
    m_tasks.emplace(Command::SEND_ALL_ENTITIES, std::move(session));
}

size_t ServerEntityManager::max_creature_sum() const {
    return PER_CREATURE_LIMITS * m_world.player_sum();
}

size_t ServerEntityManager::creature_sum() const {
    return m_creature_sum.load();
}
size_t ServerEntityManager::entity_sum() const { return m_entity_sum.load(); }

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
        Tools::set_proto_pos(s2c, base.transform.position.value);
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
    Tools::set_proto_pos(s2c, pos);
    auto packet = make_packet(*s2c);
    for (auto& s : sessions) {
        s->send(packet);
    }
}

void ServerEntityManager::handle_entity_destory(EntityID id) {
    if (m_entity_sum == 0) {
        Logger::error("entity sum is 0!");
        return;
    }
    acc a;
    if (!m_entities.find(a, id)) {
        return;
    }
    auto e = m_registry.try_get<Entity>(a->second);
    ASSERT(e);
    if (e->type == EntityType::CREATURE) {
        --m_creature_sum;
    }
    m_registry.destroy(a->second);
    m_entities.erase(a);

    --m_entity_sum;
    auto sessions = m_world.get_all_session();
    Arena arena;
    auto* s2c = Arena::Create<S2CEntityDestory>(&arena);
    s2c->set_id(id);
    auto packet = make_packet(*s2c);
    for (auto& s : sessions) {
        s->send(packet);
    }
}

} // namespace Cubed