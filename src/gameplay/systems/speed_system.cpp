#include "Cubed/gameplay/systems/speed_system.hpp"

namespace Cubed {
void SpeedSystem::update(float dt, ServerEntity& e) {
    update(dt, e.velocity, e.move_state, e.movement, e.direction, e.gravity);
}

void SpeedSystem::update(float dt, Velocity& v, MoveState& move_state,
                         Movement& movement, Direction& direction,
                         const Gravity& g) {
    // calculate speed
    if (move_state.forward || move_state.back || move_state.left ||
        move_state.right) {
        direction.value = glm::vec3(0.0f, 0.0f, 0.0f);
        v.value.x += movement.acceleration * dt;
        v.value.z += movement.acceleration * dt;
        if (v.value.x > v.max.x) {
            v.value.x = v.max.x;
        }
        if (v.value.z > v.max.z) {
            v.value.z = v.max.z;
        }
    } else {
        v.value.x += -movement.deceleration * dt;
        v.value.z += -movement.deceleration * dt;
        if (v.value.z < 0.0f) {
            v.value.z = 0.0f;
        }
        if (v.value.x < 0.0f) {
            v.value.x = 0.0f;
        }
        if (v.value.z < 0.0f && v.value.x < 0.0f) {
            direction.value = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    if (move_state.is_fly) {
        if (move_state.up) {
            v.value.y = v.max.y;
        }

        if (move_state.down) {
            v.value.y = -v.max.y;
        }

        if (!move_state.down && !move_state.up) {
            v.value.y = 0.0f;
        }
    } else {
        if (move_state.up && move_state.can_up) {
            v.value.y = movement.jump_power;
            move_state.can_up = false;
        }

        v.value.y += -g.value * dt;
    }
}

} // namespace Cubed