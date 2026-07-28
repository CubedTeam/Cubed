#pragma once
#include "Cubed/AABB.hpp"
#include "Cubed/constants.hpp"
#include "Cubed/gameplay/player.hpp"

#include <glm/glm.hpp>
#include <string>
namespace Cubed {

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
    glm::vec3 max{4.5f};
};

struct HitBoxes {
    std::vector<AABB> boxex;
};

struct Movement {
    float acceleration = DEFAULT_ACCELERATION;
    float deceleration = DEFAULT_DECELERATION;
    float jump_power = 7.5f;
};

struct Gravity {
    float value = DEFAULT_G;
};

class Entity {
public:
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

protected:
    Position m_pos;
    WalkPose m_walk_pos;
    Velocity m_velocity;
    Orientation m_angle;
    Movement m_movement;
    Gravity m_gravity;
};

} // namespace Cubed