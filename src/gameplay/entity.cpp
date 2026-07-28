#include "Cubed/gameplay/entity.hpp"

namespace Cubed {
glm::vec3& Entity::max_speed() { return m_velocity.max; }
float& Entity::acceleration() { return m_movement.acceleration; }
float& Entity::deceleration() { return m_movement.deceleration; }
float& Entity::g() { return m_gravity.value; }
void Entity::set_gait(Gait gait) { m_walk_pos.gait = gait; }
float Entity::yaw() const { return m_angle.yaw; }
float Entity::pitch() const { return m_angle.pitch; }
float& Entity::roll() { return m_angle.roll; }
float& Entity::walk_time() { return m_walk_pos.walk_time; }
Gait Entity::get_gait() const { return m_walk_pos.gait; }
} // namespace Cubed