#include "Cubed/gameplay/systems/speed_system.hpp"

#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/gameplay/ecs/server_entity.hpp"

namespace Cubed {

void SpeedSystem::update(float dt, entt::registry& registry) {
    auto view = registry.view<BaseServerCreature, MoveBoost>();

    for (auto e : view) {
        auto [creature, moveboost] = view.get<BaseServerCreature, MoveBoost>(e);
        if (moveboost.count <= moveboost.duration) {
            ++moveboost.count;
            creature.velocity.value +=
                creature.direction.value * creature.movement.acceleration;
        } else {
            creature.velocity.value -=
                creature.direction.value * creature.movement.deceleration;
        }
        creature.velocity.value.y = creature.gravity.value * dt;
    }
}

} // namespace Cubed