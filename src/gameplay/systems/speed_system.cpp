#include "Cubed/gameplay/systems/speed_system.hpp"

#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"

namespace cubed {

namespace {
void v_clamp(float& c, float max) {
    if (max < 0.0f) {
        return; //-1 = unlimited
    }
    c = std::clamp(c, -max, max);
};
} // namespace

void SpeedSystem::update(float dt, entt::registry& registry, entt::entity e) {
    update_gravity(dt, registry, e);
    update_horizontal_velocity(dt, registry, e);
}

void SpeedSystem::update_gravity(float dt, entt::registry& registry,
                                 entt::entity e) {
    if (!registry.all_of<Gravity, TickVelocity>(e)) {
        return;
    }
    const auto& gravity = registry.get<Gravity>(e);
    auto& velocity = registry.get<TickVelocity>(e);
    auto& v = velocity.value;
    v.y += -gravity.value * dt;
    v_clamp(v.y, velocity.max.y);
}
void SpeedSystem::update_horizontal_velocity(float, entt::registry& registry,
                                             entt::entity e) {
    if (!registry.all_of<TickVelocity, MoveBoost, Transform, Movement>(e)) {
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

    v_clamp(v.x, velocity.max.x);

    v_clamp(v.z, velocity.max.z);
}
} // namespace cubed
