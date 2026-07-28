#pragma once
#include "Cubed/constants.hpp"
#include "Cubed/gameplay/hitbox.hpp"
#include "Cubed/gameplay/model.hpp"
#include "Cubed/gameplay/player.hpp"

#include <glm/glm.hpp>
#include <string>
namespace Cubed {
using EntityID = uint64_t;
struct Position {
    glm::vec3 value{0.0f};
};

struct WalkPose {
    Gait gait = Gait::STOP;
    // for arm roll caculate
    float walk_time = 0.0f;
    // for sound play
    float moving_time = 0.0f;
};

struct EntityInfo {
    std::string name;
    std::string uuid;
    EntityID id;
    HitboxID hitbox;
    ModelID model;
};

struct Health {
    float hp = 20;
    float max_hp = 20;
};

struct Orientation {
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};

struct Velocity {
    glm::vec3 value{0.0f};
    glm::vec3 max{4.5f, 7.5f, 7.5f};
};

struct Movement {
    float acceleration = DEFAULT_ACCELERATION;
    float deceleration = DEFAULT_DECELERATION;
    float jump_power = 7.5f;
};

struct Gravity {
    float value = DEFAULT_G;
};

struct MoveState {
    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool down = false;
    bool up = false;

    bool is_fly = false;
    bool can_up = true;
};

struct Direction {
    glm::vec3 value{0.0f};
};

class Entity {
public:
    Entity(EntityID id, const std::string& name);

    glm::vec3& max_speed();
    float& acceleration();
    float& deceleration();
    float& g();
    void set_gait(Gait gait);
    float yaw() const;
    float pitch() const;
    float& roll();
    float& walk_time();
    Gait get_gait() const;

    Velocity& velocity();
    Position& pos();
    WalkPose& walk_pose();
    Orientation& angle();
    Movement& movement();
    Gravity& gravity();
    MoveState& move_state();
    Direction& direction();
    EntityInfo& info();

    const Velocity& velocity() const;
    const Position& pos() const;
    const WalkPose& walk_pose() const;
    const Orientation& angle() const;
    const Movement& movement() const;
    const Gravity& gravity() const;
    const MoveState& move_state() const;
    const Direction& direction() const;
    const EntityInfo& info() const;

protected:
    EntityInfo m_info;
    Position m_pos;
    WalkPose m_walk_pose;
    Velocity m_velocity;
    Orientation m_angle;
    Movement m_movement;
    Gravity m_gravity;
    MoveState m_move_state;
    Direction m_direction;
};

} // namespace Cubed