#include "Cubed/gameplay/entity.hpp"

#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/render/model_manager.hpp"
namespace Cubed {

Entity::Entity(EntityID id, const std::string& name) {
    m_info.id = id;
    m_info.name = name;
    auto h = ModelManager::model(name);
    m_info.model = h.id;
    auto b = HitboxManager::hitbox(name);
    m_info.hitbox = b.id;
}

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
EntityInfo& Entity::info() { return m_info; }

const Velocity& Entity::velocity() const { return m_velocity; }
const Position& Entity::pos() const { return m_pos; }
const WalkPose& Entity::walk_pose() const { return m_walk_pose; }
const Orientation& Entity::angle() const { return m_angle; }
const Movement& Entity::movement() const { return m_movement; }
const Gravity& Entity::gravity() const { return m_gravity; }
const MoveState& Entity::move_state() const { return m_move_state; }
const Direction& Entity::direction() const { return m_direction; }
const EntityInfo& Entity::info() const { return m_info; }

} // namespace Cubed