#include "Cubed/gameplay/client_entity_manager.hpp"

#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/gameplay/creatures/pig.hpp"
#include "Cubed/gameplay/ecs/client_entity.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/render/model_manager.hpp"
#include "Cubed/tools/cubed_assert.hpp"
namespace Cubed {
ClientEntityManager::ClientEntityManager(ClientWorld& world) : m_world(world) {}
void ClientEntityManager::update() { handle_task(); }

void ClientEntityManager::init() {
    m_factories.emplace("cubed:pig", [this](EntityID id) {
        BaseClientCreature c;
        c.model = ModelManager::instance().get_model_id("cubed:pig");
        create_entity_in_registry(id, Entity{id}, EntityInfo{"cubed:pig", ""},
                                  std::move(c), PigTag{});
    });
}
// not thread safe
void ClientEntityManager::add_entity(EntityID id, std::string_view name,
                                     const glm::vec3& pos) {
    ASSERT(m_factories.contains(name));
    m_factories[name](id);
    acc a;
    ASSERT(m_entities.find(a, id));
    auto* c = m_registry.try_get<BaseClientCreature>(a->second);
    ASSERT(c);
    c->transform.position.value = pos;
}

void ClientEntityManager::receive_entity_create(S2CEntityCreate& s2c) {
    EntityCreateElement c;
    c.id = s2c.id();
    c.name = s2c.name();
    c.pos = {s2c.pos().x(), s2c.pos().y(), s2c.pos().z()};
    m_tasks.emplace(Command::CREATE, std::move(c));
}

void ClientEntityManager::destory(EntityID id) {
    m_tasks.emplace(Command::DESTORY, id);
}

void ClientEntityManager::handle_task() {
    TaskPair pair;
    while (m_tasks.try_pop(pair)) {
        switch (pair.first) {
        case Command::CREATE: {
            auto* p = std::get_if<EntityCreateElement>(&pair.second);
            ASSERT(p);
            add_entity(p->id, p->name, p->pos);

        } break;
        case Command::DESTORY: {
            auto* p = std::get_if<EntityID>(&pair.second);
            ASSERT(p);
            handle_entity_destory(*p);
        } break;
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