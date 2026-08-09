#include "Cubed/gameplay/client_entity_manager.hpp"

#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/gameplay/creatures/creature_manager.hpp"
#include "Cubed/gameplay/creatures/pig.hpp"
#include "Cubed/gameplay/ecs/client_entity.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/render/model_manager.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/math_tools.hpp"
#include "Cubed/tools/net_utils.hpp"
#include "Cubed/tools/time_tools.hpp"
using namespace google::protobuf;

namespace Cubed {

namespace {
constexpr double ENTITY_RENDER_DELAY_MS = 100.0; // two tick time
constexpr size_t ENTITY_SNAPSHOT_MAX = 16;

ClientEntitySnapshot
interpolate_snapshot(const std::deque<ClientEntitySnapshot>& history,
                     double render_time) {

    if (history.empty()) {
        return {};
    }

    if (history.size() == 1) {
        return history.front();
    }

    auto upper = std::lower_bound(
        history.begin(), history.end(), render_time,
        [](const ClientEntitySnapshot& s, double t) { return s.time_ms < t; });

    if (upper == history.end()) {
        return history.back();
    }

    if (upper == history.begin()) {
        return *upper;
    }

    auto lower = upper - 1;

    double span = upper->time_ms - lower->time_ms;

    float t = span > 0.0 ? float((render_time - lower->time_ms) / span) : 1.0f;

    t = std::clamp(t, 0.0f, 1.0f);

    ClientEntitySnapshot out;

    out.pos = glm::mix(lower->pos, upper->pos, t);

    glm::vec3 mixed = glm::mix(lower->dir, upper->dir, t);

    out.dir =
        glm::length(mixed) > 1e-6f ? glm::normalize(mixed) : glm::vec3{0.0f};
    return out;
}
} // namespace

ClientEntityManager::ClientEntityManager(ClientWorld& world) : m_world(world) {}
void ClientEntityManager::update(float dt) {
    handle_task(dt);
    {
        auto view = m_registry.view<ClientEntityState, RenderTransform>();
        double render_time = static_cast<double>(Tools::get_time_ticks()) -
                             ENTITY_RENDER_DELAY_MS;
        for (auto e : view) {
            auto& state = view.get<ClientEntityState>(e);
            auto& r = view.get<RenderTransform>(e);
            auto snap = interpolate_snapshot(state.history, render_time);
            r.position.value = snap.pos;
            r.direction.value = snap.dir;
        }
    }

    auto view = m_registry.view<BaseClientCreature>();
    for (auto e : view) {
        auto& c = view.get<BaseClientCreature>(e);
        if (c.pose.gait == Gait::STOP) {
            c.pose.walk_time = 0.0f;
        } else {
            c.pose.walk_time += dt;
        }
    }

    player_sound(dt);
}

void ClientEntityManager::init() {
    m_random.init(std::random_device()());
    m_factories.emplace("cubed:pig", [this](EntityID id) {
        BaseClientCreature c;
        c.model = ModelManager::instance().get_model_id("cubed:pig");
        float next_call_time = m_random.random_float(8.0f, 25.0f);
        create_entity_in_registry(
            id, Entity{id, EntityType::CREATURE}, EntityInfo{"cubed:pig", ""},
            std::move(c), PigTag{}, RenderTransform{},
            SoundTime{next_call_time}, ClientEntityState{});
    });
}
// not thread safe
void ClientEntityManager::handle_entity_create(EntityID id,
                                               std::string_view name,
                                               const glm::vec3& pos) {
    ASSERT(m_factories.contains(name));
    m_factories[name](id);
    acc a;
    bool found = m_entities.find(a, id);
    ASSERT(found);
    auto* c = m_registry.try_get<BaseClientCreature>(a->second);
    ASSERT(c);
    c->transform.position.value = pos;
    auto* r = m_registry.try_get<RenderTransform>(a->second);
    ASSERT(r);
    r->position.value = pos;
    auto* s = m_registry.try_get<ClientEntityState>(a->second);
    ASSERT(s);
    s->history.emplace_back(static_cast<double>(Tools::get_time_ticks()), pos,
                            glm::vec3{0, 0, 0});
}

void ClientEntityManager::handle_entity_update(UpdateInfo& info, float) {
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

    auto state = m_registry.try_get<ClientEntityState>(e);
    ASSERT(state);
    state->history.push_back({static_cast<double>(Tools::get_time_ticks()),
                              info.pos, info.direction});
    while (state->history.size() > ENTITY_SNAPSHOT_MAX) {
        state->history.pop_front();
    }
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

void ClientEntityManager::receive_entity_update(const S2CEntityUpdate& msg) {
    UpdateInfo e;
    e.id = msg.id();
    e.pos = Tools::get_net_vec3(msg.pos());
    e.direction = Tools::get_net_vec3(msg.direction());
    e.gait = get_gait_from_id(msg.gait());
    m_tasks.emplace(Command::UPDATE, std::move(e));
}

void ClientEntityManager::receive_entity_update(S2CEntityUpdateBatch& msg) {
    for (auto& u : msg.updates()) {
        receive_entity_update(u);
    }
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

void ClientEntityManager::handle_task(float dt) {
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
            handle_entity_update(*p, dt);
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

void ClientEntityManager::player_sound(float dt) {
    auto& audio = m_world.get_audio();
    auto player_pos = m_world.player_manager().get_local().get_player_pos();
    auto view = m_registry.view<EntityInfo, BaseClientCreature, SoundTime>();

    for (auto e : view) {
        const auto [info, creature] =
            view.get<EntityInfo, BaseClientCreature>(e);
        if (Math::distance2(player_pos, creature.transform.position.value) >
            10 * 10) {
            continue;
        }
        auto& sound_time = view.get<SoundTime>(e);
        sound_time.next_call_time -= dt;
        if (sound_time.next_call_time <= 0.0f) {

            auto data = CreatureManager::data(info.name);
            if (data.sound.call) {
                audio.play_3d(data.sound.call->full_path(),
                              creature.transform.position.value, true, false);
            }
            sound_time.next_call_time = m_random.random_float(8.0f, 25.0f);
        }
    }
}

} // namespace Cubed