#include "Cubed/gameplay/server/server_entity_manager.hpp"

#include "Cubed/gameplay/creatures/pig.hpp"
#include "Cubed/gameplay/ecs/collision.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/gait.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/gameplay/server/session.hpp"
#include "Cubed/gameplay/systems/physical_system.hpp"
#include "Cubed/gameplay/systems/speed_system.hpp"
#include "Cubed/gameplay/systems/wander_ai_system.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/proto_utils.hpp"

#include <algorithm>
#include <tracy/Tracy.hpp>
using namespace google::protobuf;

namespace cubed {
ServerEntityManager::ServerEntityManager(ServerWorld& world) : m_world(world) {}

void ServerEntityManager::init() {

    m_factories.try_emplace("cubed:pig", [this](EntityID id) {
        Collider hitbox{HitboxManager::instance().get_hitbox_id("cubed:pig")};

        Gravity gravity{pig_defaults::GRAVITY};
        Movement move;
        move.acceleration = pig_defaults::ACCELERATION;
        move.deceleration = pig_defaults::DECELERATION;

        TickVelocity velocity;

        velocity.max.x = velocity.max.z = pig_defaults::MAX_SPEED;

        return create_entity_in_factory(
            id, Entity{id, EntityType::CREATURE},
            EntityInfo{"cubed:pig", std::nullopt}, Transform{}, PigTag{},
            AIBase{}, WanderAITag{}, MoveBoost{}, std::move(hitbox),
            std::move(gravity), std::move(move), std::move(velocity));
    });

    m_storage = std::make_unique<EntityStorage>(*m_world.world_storage());
    auto entities = m_storage->load_all();

    for (auto& data : entities) {
        m_next = std::max(m_next, data.id + 1);
        add_dormant(std::move(data));
    }

    Logger::info("ServerEntityManager initialization successful.");
}

void ServerEntityManager::stop() {
    save_all_entities(true);
    Logger::info("ServerEntityManager stopped successful.");
}

void ServerEntityManager::add_dormant(EntityStorageData data) {
    auto chunk_pos = get_chunk_pos(data.pos.x, data.pos.z);

    m_dormant_entities[chunk_pos].emplace_back(std::move(data));
}

void ServerEntityManager::activate_chunk(ChunkPos pos) {
    auto node = m_dormant_entities.extract(pos);
    if (node.empty()) {
        return;
    }
    for (auto& data : node.mapped()) {
        auto factory = m_factories.find(data.name);
        if (factory == m_factories.end()) {
            Logger::error("Unknown entity type {}", data.name);
            add_dormant(std::move(data));
            continue;
        }

        EntityID id = factory->second(data.id);

        acc a;
        if (!m_entities.find(a, id)) {
            Logger::error("Entity {} created but not found", id);
            add_dormant(std::move(data));
            continue;
        }
        auto* transform = m_registry.try_get<Transform>(a->second);
        auto* entity = m_registry.try_get<Entity>(a->second);
        if (transform) {
            transform->position.value = data.pos;
            transform->direction.value = data.dir;
            if (entity && entity->type == EntityType::CREATURE) {
                ++m_creature_sum;
            }
        }
        handle_entity_create(data.id, data.name, data.pos);
    }
}

void ServerEntityManager::update() {
    ZoneScopedN("Server Entity update");
    handle_task();

    auto pool = m_world.get_compute_pool();
    if (!pool) {
        return;
    }
    auto sessions = m_world.get_all_session();
    std::vector<entt::entity> entities;
    for (auto e : m_registry.view<entt::entity>()) {
        entities.push_back(e);
    }
    tbb::concurrent_vector<EntitySendData> send_data;
    // parallel block touches disjoint entities only;
    // structural registry changes stay on the server thread via m_tasks.
    parallel_do(*pool, entities.begin(), entities.end(), pool->thread_sum(),
                [this, &send_data](entt::entity e) {
                    if (!m_registry.all_of<Entity>(e)) {
                        Logger::error("Entity don't have Entity component");
                        return;
                    }
                    auto c = m_registry.try_get<Transform>(e);
                    ASSERT(c);
                    auto entity = m_registry.try_get<Entity>(e);
                    ASSERT(entity);
                    if (!m_world.is_chunk_active(c->position.value)) {
                        unload(entity->id);
                        return;
                    }
                    update_ai(e);
                    update_move(e);
                    update_send(e, send_data);
                });
    if (!send_data.empty()) {
        Arena arena;
        auto* msg = Arena::Create<protocol::S2CEntityUpdateBatch>(&arena);
        for (auto& data : send_data) {
            auto* u = msg->add_updates();
            u->set_id(data.id);
            tools::set_proto_pos(u, data.pos);
            tools::set_proto_vec3(u->mutable_direction(), data.dir);
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

    if (!m_registry.all_of<Entity, Transform, TickVelocity>(e)) {
        return;
    }

    const auto [entity, transform, v] =
        m_registry.get<Entity, Transform, TickVelocity>(e);
    EntitySendData data;
    data.id = entity.id;
    data.pos = transform.position.value;
    data.dir = transform.direction.value;
    if (v.value.x * v.value.x + v.value.z * v.value.z > 1e-4f) {
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
        auto data = build_entity_storage_data(a->second);
        if (!data) {
            return;
        }
        if (m_world.is_chunk_active(data->pos)) {
            return;
        }

        if (!m_storage->save(*data)) {
            Logger::error("Can't unload entity {}, save failed", id);
            return;
        }
        auto e = m_registry.try_get<Entity>(a->second);
        ASSERT(e);
        if (e->type == EntityType::CREATURE) {
            --m_creature_sum;
        }

        add_dormant(std::move(*data));

        m_registry.destroy(a->second);
        m_entities.erase(a);
        --m_entity_sum;
    }

    auto sessions = m_world.get_all_session();
    Arena arena;
    auto* s2c = Arena::Create<protocol::S2CEntityDestroy>(&arena);
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

    if (!m_registry.all_of<Entity, Transform, EntityInfo>(e)) {
        return std::nullopt;
    }

    EntityStorageData data;
    auto entity = m_registry.try_get<Entity>(e);
    ASSERT(entity);
    data.id = entity->id;
    auto transform = m_registry.try_get<Transform>(e);
    if (transform) {
        data.dir = transform->direction.value;
        data.pos = transform->position.value;
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
        case Command::DESTROY: {
            auto* c = std::get_if<EntityID>(&pair.second);
            ASSERT(c);
            handle_entity_destroy(*c);
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

void ServerEntityManager::destroy(EntityID id) {
    m_tasks.emplace(Command::DESTROY, id);
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
        auto t = m_registry.try_get<Transform>(c->second);
        ASSERT(t);
        t->position.value = pos;
    }

    handle_entity_create(e, name, pos);
}

void ServerEntityManager::unload(EntityID id) {
    m_tasks.emplace(Command::UNLOAD, id);
}

void ServerEntityManager::send_all_entities(std::shared_ptr<Session>& session) {
    auto view = m_registry.view<Entity, EntityInfo, Transform>();
    for (auto& entity : view) {
        auto [e, info, transform] =
            view.get<Entity, EntityInfo, Transform>(entity);
        Arena arena;
        auto* s2c = Arena::Create<protocol::S2CEntityCreate>(&arena);
        s2c->set_id(e.id);
        s2c->set_name(info.name);
        tools::set_proto_pos(s2c, transform.position.value);
        session->send(make_packet(*s2c));
    }
}

void ServerEntityManager::handle_entity_create(EntityID id,
                                               std::string_view name,
                                               const glm::vec3& pos) {
    auto sessions = m_world.get_all_session();

    Arena arena;
    auto* s2c = Arena::Create<protocol::S2CEntityCreate>(&arena);
    s2c->set_id(id);
    s2c->set_name(name);
    tools::set_proto_pos(s2c, pos);
    auto packet = make_packet(*s2c);
    for (auto& s : sessions) {
        s->send(packet);
    }
}

bool ServerEntityManager::destroy_internal(EntityID id) {
    {
        acc a;
        if (m_entities.find(a, id)) {
            // active
            auto e = m_registry.try_get<Entity>(a->second);
            ASSERT(e);

            if (!m_storage->remove(id)) {
                Logger::error("Can't destroy dormant entity {}", id);
                return false;
            }
            if (e->type == EntityType::CREATURE) {
                --m_creature_sum;
            }
            m_registry.destroy(a->second);
            m_entities.erase(a);
            --m_entity_sum;
            return true;
        }
    }
    for (auto chunk = m_dormant_entities.begin();
         chunk != m_dormant_entities.end(); ++chunk) {
        auto& entities = chunk->second;
        auto entity = std::ranges::find(entities, id, &EntityStorageData::id);

        if (entity == entities.end()) {
            continue;
        }

        if (!m_storage->remove(id)) {
            Logger::error("Can't destroy dormant entity {}", id);
            return false;
        }

        entities.erase(entity);
        if (entities.empty()) {
            m_dormant_entities.erase(chunk);
        }
        return true;
    }
    return false;
}

void ServerEntityManager::handle_entity_destroy(EntityID id) {

    if (!destroy_internal(id)) {
        return;
    }

    auto sessions = m_world.get_all_session();
    Arena arena;
    auto* s2c = Arena::Create<protocol::S2CEntityDestroy>(&arena);
    s2c->set_id(id);
    auto packet = make_packet(*s2c);
    for (auto& s : sessions) {
        s->send(packet);
    }
}

} // namespace cubed
