#pragma once
#include "Cubed/AABB.hpp"
#include "Cubed/gameplay/player.hpp"

#include <glm/glm.hpp>
#include <string>
namespace Cubed {

using ModelID = uint32_t;

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

struct Model {
    ModelID id;
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
};

struct HitBoxes {
    std::vector<AABB> boxex;
};

} // namespace Cubed