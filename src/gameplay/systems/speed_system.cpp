#include "Cubed/gameplay/systems/speed_system.hpp"

#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/gameplay/ecs/server_entity.hpp"

namespace cubed {

void SpeedSystem::update(float dt, entt::registry& registry, entt::entity e) {
    if (!registry.all_of<BaseServerCreature, MoveBoost>(e)) {
        return;
    }

    auto [creature, moveboost] = registry.get<BaseServerCreature, MoveBoost>(e);
    auto& v = creature.velocity.value;
    if (moveboost.count <= moveboost.duration) {
        ++moveboost.count;
        v +=
            creature.transform.direction.value * creature.movement.acceleration;
    } else {
        // Decelerated by friction in all directions
        auto decay = [](float& c, float d) {
            if (c > 0.0f)
                c = std::max(0.0f, c - d);
            else if (c < 0.0f)
                c = std::min(0.0f, c + d);
        };
        decay(v.x, creature.movement.deceleration);
        decay(v.z, creature.movement.deceleration);
    }
    v.y += -creature.gravity.value * dt;
    auto v_clamp = [](float& c, float max) {
        if (max < 0.0f) {
            return; //-1 = unlimited
        }
        c = std::clamp(c, -max, max);
    };
    v_clamp(v.x, creature.velocity.max.x);
    v_clamp(v.y, creature.velocity.max.y);
    v_clamp(v.z, creature.velocity.max.z);
}

} // namespace cubed
