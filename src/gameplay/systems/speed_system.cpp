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
        } else if (glm::dot(creature.velocity.value, creature.velocity.value) >=
                   0.001f) {
            creature.velocity.value -=
                creature.direction.value * creature.movement.deceleration;
        } else {
            creature.velocity.value = glm::vec3(0.0f);
        }
        creature.velocity.value.y += -creature.gravity.value * dt;
        auto v_clamp = [](float& v, float max) {
            auto temp = std::abs(v);
            max = std::abs(max);
            temp = std::clamp(temp, 0.0f, max);

            v = temp;
        };
        v_clamp(creature.velocity.value.x, creature.velocity.max.x);
        v_clamp(creature.velocity.value.y, creature.velocity.max.y);
        v_clamp(creature.velocity.value.z, creature.velocity.max.z);
    }
}

} // namespace Cubed