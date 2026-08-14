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

    m_factories.try_emplace("cubed:pig", [this](EntityID id) {
        BaseServerCreature c;
        c.hitbox = HitboxManager::instance().get_hitbox_id("cubed:pig");
        c.gravity.value = PigDefaults::GRAVITY;
        c.movement.acceleration = PigDefaults::ACCELERATION;
        c.movement.deceleration = PigDefaults::DECELERATION;
        c.velocity.max.x = c.velocity.max.z = PigDefaults::MAX_SPEED;
        return create_entity_in_factory(
            id, Entity{id, EntityType::CREATURE}, EntityInfo{"cubed:pig", ""},
            std::move(c), PigTag{}, AIBase{}, WanderAITag{}, MoveBoost{});
    });

    m_storage = std::make_unique<EntityStorage>(*m_world.world_storage());
    m_storage->load_all_entities(*this);
    Logger::info("ServerEntityManager initialization successful.");
}

void ServerEntityManager::stop() {
    save_all_entities(true);
    Logger::info("ServerEntityManagerstopped successful.");
}

void ServerEntityManager::add_entity_on_init(EntityStorageData& data) {
    ASSERT(m_factories.contains(data.name));
    auto e = m_factories[data.name](data.id);

    acc c;
    if (m_entities.find(c, e)) {
        auto t = m_registry.try_get<BaseServerCreature>(c->second);
        if (t) {
            ++m_creature_sum;
            t->transform.position.value = data.pos;
            t->transform.direction.value = data.dir;
        }
    }
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
            const auto& entity = m_registry.get<Entity>(e);
            if (!m_world.get_chunk_ref_count(c.transform.position.value)) {

                ActiveIds::accessor acc;
                if (m_active_ids.find(acc, entity.id)) {
                    unload(entity.id);
                }

                return;
            }
            m_active_ids.emplace(entity.id, std::monostate{});
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

void ServerEntityManager::save_all() {
    auto view = m_registry.view<Entity, EntityInfo>();
    std::vector<EntityStorageData> datas;
    for (auto e : view) {
        auto data = build_entity_storage_data(e);
        if (data) {
            datas.emplace_back(std::move(*data));
        }
    }

    m_storage->save_batch(datas);
}

void ServerEntityManager::save(EntityID id) {
    auto data = build_entity_storage_data(id);
    if (!data) {
        return;
    }

    m_storage->save(*data);
}
void ServerEntityManager::save(entt::entity e) {
    auto data = build_entity_storage_data(e);
    if (!data) {
        return;
    }
    m_storage->save(*data);
}

void ServerEntityManager::unload_internal(EntityID id) {

    if (m_entity_sum == 0) {
        Logger::error("entity sum is 0!");
        return;
    }
    {
        acc a;
        if (!m_entities.find(a, id)) {
            return;
        }
        auto e = m_registry.try_get<Entity>(a->second);
        ASSERT(e);
        if (e->type == EntityType::CREATURE) {
            --m_creature_sum;
        }

        save(a->second);

        m_registry.destroy(a->second);
        m_active_ids.erase(id);
        m_entities.erase(a);
    }

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

std::optional<EntityStorageData>
ServerEntityManager::build_entity_storage_data(EntityID id) {
    entt::entity e;
    {
        EntityMap::const_accessor cacc;
        if (!m_entities.find(cacc, id)) {
            Logger::error("Can't find entity {} id entities map", id);
            return std::nullopt;
        }
        e = cacc->second;
    }
    return build_entity_storage_data(e);
}

std::optional<EntityStorageData>
ServerEntityManager::build_entity_storage_data(entt::entity e) {
    EntityStorageData data;
    auto entity = m_registry.try_get<Entity>(e);
    ASSERT(entity);
    data.id = entity->id;
    auto base = m_registry.try_get<BaseServerCreature>(e);
    if (base) {
        data.dir = base->transform.direction.value;
        data.pos = base->transform.position.value;
    }
    auto info = m_registry.try_get<EntityInfo>(e);
    ASSERT(info);
    data.name = info->name;

    return data;
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
        case Command::SAVE_ALL: {
            save_all();
        } break;
        case Command::UNLOAD: {
            auto* c = std::get_if<EntityID>(&pair.second);
            ASSERT(c);
            unload_internal(*c);
        }; break;
        }
    }
}

void ServerEntityManager::add_creature(std::string_view name,
                                       const glm::vec3& world_pos) {
    if (m_creature_sum.fetch_add(1) >= max_creature_sum()) {
        m_creature_sum.fetch_sub(1);
        return;
    }

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

void ServerEntityManager::save_all_entities(bool immediately) {
    if (immediately) {
        save_all();
    } else {
        m_tasks.emplace(Command::SAVE_ALL, std::monostate{});
    }
}

size_t ServerEntityManager::max_creature_sum() const {
    return PER_CREATURE_LIMITS * m_world.player_sum();
}

size_t ServerEntityManager::creature_sum() const {
    return m_creature_sum.load();
}
size_t ServerEntityManager::entity_sum() const { return m_entity_sum.load(); }

EntityID ServerEntityManager::get_next_value() const { return m_next; }
void ServerEntityManager::set_next_value(EntityID id) { m_next = id; }
void ServerEntityManager::create_entity(std::string_view name,
                                        const glm::vec3& pos) {
    ASSERT(m_factories.contains(name));
    auto e = m_factories[name](m_next++);
    acc c;
    if (m_entities.find(c, e)) {
        auto t = m_registry.try_get<BaseServerCreature>(c->second);
        ASSERT(t);
        t->transform.position.value = pos;
    }

    handle_entity_create(e, name, pos);
}

void ServerEntityManager::unload(EntityID id) {
    m_tasks.emplace(Command::UNLOAD, id);
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
    {
        acc a;
        if (!m_entities.find(a, id)) {
            return;
        }
        auto e = m_registry.try_get<Entity>(a->second);
        ASSERT(e);
        if (e->type == EntityType::CREATURE) {
            --m_creature_sum;
        }

        m_storage->remove(id);

        m_registry.destroy(a->second);
        m_active_ids.erase(id);
        m_entities.erase(a);
    }

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