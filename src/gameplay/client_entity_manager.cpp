#include "Cubed/gameplay/client_entity_manager.hpp"

#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/gameplay/creatures/pig.hpp"
#include "Cubed/gameplay/ecs/client_entity.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/render/model_manager.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/net_utils.hpp"

using namespace google::protobuf;

namespace Cubed {
ClientEntityManager::ClientEntityManager(ClientWorld& world) : m_world(world) {}
void ClientEntityManager::update(float dt) {
    handle_task();

    auto view = m_registry.view<BaseClientCreature>();
    for (auto e : view) {
        auto& c = view.get<BaseClientCreature>(e);
        if (c.pose.gait == Gait::STOP) {
            c.pose.walk_time = 0.0f;
        } else {
            c.pose.walk_time += dt;
        }
    }
}

void ClientEntityManager::init() {
    m_factories.emplace("cubed:pig", [this](EntityID id) {
        BaseClientCreature c;
        c.model = ModelManager::instance().get_model_id("cubed:pig");
        create_entity_in_registry(id, Entity{id}, EntityInfo{"cubed:pig", ""},
                                  std::move(c), PigTag{}, RenderTransform{});
    });
}
// not thread safe
void ClientEntityManager::handle_entity_create(EntityID id,
                                               std::string_view name,
                                               const glm::vec3& pos) {
    ASSERT(m_factories.contains(name));
    m_factories[name](id);
    acc a;
    ASSERT(m_entities.find(a, id));
    auto* c = m_registry.try_get<BaseClientCreature>(a->second);
    ASSERT(c);
    c->transform.position.value = pos;
    auto* r = m_registry.try_get<RenderTransform>(a->second);
    ASSERT(r);
    r->position.value = pos;
}

void ClientEntityManager::handle_entity_update(UpdateInfo& info) {
    entt::entity e;
    {
        cacc a;
        if (m_entities.find(a, info.id)) {
            e = a->second;
        } else {
            return;
        }
    }
    auto creature = m_registry.try_get<BaseClientCreature>(e);
    ASSERT(creature);
    creature->transform.position.value = info.pos;
    creature->transform.direction.value = info.direction;
    creature->pose.gait = info.gait;
    auto r = m_registry.try_get<RenderTransform>(e);
    ASSERT(r);
    r->direction.value =
        glm::mix(r->direction.value, creature->transform.direction.value, 0.15);
    r->position.value =
        glm::mix(r->position.value, creature->transform.position.value, 0.15);
}

void ClientEntityManager::receive_entity_create(S2CEntityCreate& s2c) {
    EntityCreateElement c{};
    c.id = s2c.id();
    c.name = s2c.name();
    c.pos = {s2c.pos().x(), s2c.pos().y(), s2c.pos().z()};
    m_tasks.emplace(Command::CREATE, std::move(c));
}

void ClientEntityManager::receive_entity_destory(EntityID id) {
    m_tasks.emplace(Command::DESTORY, id);
}

void ClientEntityManager::receive_entity_update(S2CEntityUpdate& msg) {
    UpdateInfo e;
    e.id = msg.id();
    e.pos = Tools::get_net_vec3(msg.pos());
    e.direction = Tools::get_net_vec3(msg.direction());
    e.gait = get_gait_from_id(msg.gait());
    m_tasks.emplace(Command::UPDATE, std::move(e));
}

void ClientEntityManager::destory(EntityID id) {
    auto client = m_world.get_client();
    Arena arena;
    auto* msg = Arena::Create<C2SEntityDestoryRequest>(&arena);
    msg->set_id(id);
    msg->set_uuid(m_world.get_player().get_uuid());
    client->send(make_packet(*msg));
}
void ClientEntityManager::create(std::string_view name, const glm::vec3& pos) {
    auto client = m_world.get_client();
    Arena arena;
    auto* msg = Arena::Create<C2SEntityCreateRequest>(&arena);
    msg->set_name(name);
    msg->set_uuid(m_world.get_player().get_uuid());
    Tools::set_net_pos(msg, pos);
    client->send(make_packet(msg));
}

void ClientEntityManager::handle_task() {
    TaskPair pair;
    while (m_tasks.try_pop(pair)) {
        switch (pair.first) {
        case Command::CREATE: {
            auto* p = std::get_if<EntityCreateElement>(&pair.second);
            ASSERT(p);
            handle_entity_create(p->id, p->name, p->pos);

        } break;
        case Command::DESTORY: {
            auto* p = std::get_if<EntityID>(&pair.second);
            ASSERT(p);
            handle_entity_destory(*p);
        } break;
        case Command::UPDATE: {
            auto* p = std::get_if<UpdateInfo>(&pair.second);
            ASSERT(p);
            handle_entity_update(*p);
        }
        }
    }
}

void ClientEntityManager::handle_entity_destory(EntityID id) {
    acc a;
    if (!m_entities.find(a, id)) {
        return;
    }
    m_registry.destroy(a->second);
    m_entities.erase(a);
}

const entt::registry& ClientEntityManager::get_registry() const {
    return m_registry;
}
} // namespace Cubed