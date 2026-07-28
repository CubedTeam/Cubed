#include "Cubed/gameplay/systems/speed_system.hpp"

namespace Cubed {
void SpeedSystem::update(float dt, Entity& e) {
    auto& m_velocity = e.velocity();
    auto& m_move_state = e.move_state();
    auto& m_movement = e.movement();
    auto& direction = e.direction();
    auto& m_gravity = e.gravity();
    // calculate speed
    if (m_move_state.forward || m_move_state.back || m_move_state.left ||
        m_move_state.right) {
        direction.value = glm::vec3(0.0f, 0.0f, 0.0f);
        m_velocity.value.x += m_movement.acceleration * dt;
        m_velocity.value.z += m_movement.acceleration * dt;
        if (m_velocity.value.x > m_velocity.max.x) {
            m_velocity.value.x = m_velocity.max.x;
        }
        if (m_velocity.value.z > m_velocity.max.z) {
            m_velocity.value.z = m_velocity.max.z;
        }
    } else {
        m_velocity.value.x += -m_movement.deceleration * dt;
        m_velocity.value.z += -m_movement.deceleration * dt;
        if (m_velocity.value.z < 0.0f) {
            m_velocity.value.z = 0.0f;
        }
        if (m_velocity.value.x < 0.0f) {
            m_velocity.value.x = 0.0f;
        }
        if (m_velocity.value.z < 0.0f && m_velocity.value.x < 0.0f) {
            direction.value = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    if (m_move_state.is_fly) {
        if (m_move_state.up) {
            m_velocity.value.y = m_velocity.max.y;
        }

        if (m_move_state.down) {
            m_velocity.value.y = -m_velocity.max.y;
        }

        if (!m_move_state.down && !m_move_state.up) {
            m_velocity.value.y = 0.0f;
        }
    } else {
        if (m_move_state.up && m_move_state.can_up) {
            m_velocity.value.y = m_movement.jump_power;
            m_move_state.can_up = false;
        }

        m_velocity.value.y += -m_gravity.value * dt;
    }
}
} // namespace Cubed