#include "Cubed/gameplay/server_entity_manager.hpp"

#include "Cubed/gameplay/creatures/pig.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/server_entity.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/gameplay/server_world.hpp"
#include "Cubed/gameplay/session.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/net_utils.hpp"
using namespace google::protobuf;

namespace Cubed {
ServerEntityManager::ServerEntityManager(ServerWorld& world) : m_world(world) {}

void ServerEntityManager::init() {
    m_factories.try_emplace("cubed:pig", [this]() {
        BaseServerCreature c;
        c.hitbox = HitboxManager::instance().get_hitbox_id("cubed:pig");
        return add_entity(Entity{m_next}, EntityInfo{"cubed:pig", ""},
                          std::move(c), PigTag{});
    });
}

void ServerEntityManager::add_entity(std::string_view name,
                                     const glm::vec3& pos) {
    ASSERT(m_factories.contains(name));
    auto e = m_factories[name]();
    acc c;
    if (m_entities.find(c, e)) {
        auto t = m_registry.try_get<BaseServerCreature>(c->second);
        ASSERT(t);
        t->transform.position.value = pos;
    }
    send_entity_create(e, name, pos);
}

void ServerEntityManager::send_entity_create(EntityID id, std::string_view name,
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

} // namespace Cubed