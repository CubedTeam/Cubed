#include "Cubed/gameplay/systems/speed_system.hpp"

#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"

namespace cubed {

void SpeedSystem::update(float dt, entt::registry& registry, entt::entity e) {
    if (!registry.all_of<TickVelocity, MoveBoost, Transform, Movement, Gravity>(
            e)) {
        return;
    }

    auto [velocity, moveboost, transform, movement] =
        registry.get<TickVelocity, MoveBoost, Transform, Movement>(e);
    auto& v = velocity.value;
    if (moveboost.count <= moveboost.duration) {
        ++moveboost.count;
        v += transform.direction.value * movement.acceleration;
    } else {
        // Decelerated by friction in all directions
        auto decay = [](float& c, float d) {
            if (c > 0.0f)
                c = std::max(0.0f, c - d);
            else if (c < 0.0f)
                c = std::min(0.0f, c + d);
        };
        decay(v.x, movement.deceleration);
        decay(v.z, movement.deceleration);
    }
    const auto& gravity = registry.get<Gravity>(e);
    v.y += -gravity.value * dt;
    auto v_clamp = [](float& c, float max) {
        if (max < 0.0f) {
            return; //-1 = unlimited
        }
        c = std::clamp(c, -max, max);
    };
    v_clamp(v.x, velocity.max.x);
    v_clamp(v.y, velocity.max.y);
    v_clamp(v.z, velocity.max.z);
}

} // namespace cubed
