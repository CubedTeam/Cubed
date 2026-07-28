#include "Cubed/gameplay/entity.hpp"

namespace Cubed {
glm::vec3& Entity::max_speed() { return m_velocity.max; }
float& Entity::acceleration() { return m_movement.acceleration; }
float& Entity::deceleration() { return m_movement.deceleration; }
float& Entity::g() { return m_gravity.value; }
void Entity::set_gait(Gait gait) { m_walk_pose.gait = gait; }
float Entity::yaw() const { return m_angle.yaw; }
float Entity::pitch() const { return m_angle.pitch; }
float& Entity::roll() { return m_angle.roll; }
float& Entity::walk_time() { return m_walk_pose.walk_time; }
Gait Entity::get_gait() const { return m_walk_pose.gait; }

Velocity& Entity::velocity() { return m_velocity; }
Position& Entity::pos() { return m_pos; }
WalkPose& Entity::walk_pose() { return m_walk_pose; }
Orientation& Entity::angle() { return m_angle; }
Movement& Entity::movement() { return m_movement; }
Gravity& Entity::gravity() { return m_gravity; }
MoveState& Entity::move_state() { return m_move_state; }
Direction& Entity::direction() { return m_direction; }
} // namespace Cubed