#pragma once
#include "Cubed/AABB.hpp"
#include "Cubed/gameplay/player.hpp"

#include <glm/glm.hpp>
#include <string>
namespace Cubed {
struct Transform {
    glm::vec3 pos{0, 0, 0};
    glm::vec3 render_pos{0, 0, 0};
    Gait gait = Gait::STOP;
    float walk_time = 0.0f;
    float moving_time = 0.0f;
};

struct EntityInfo {
    std::string name;
    std::string uuid;
};

struct Model {
    std::string name;
};

struct Health {
    float hp = 0;
    float max_hp = 20;
};

struct ViewAngles {
    float render_yaw = 0.0f;
    float yaw = 0.0f;
    float render_pitch = 0.0f;
    float pitch = 0.0f;
    float angle = 0.0f;
};

struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;
};

struct HitBoxes {
    std::vector<AABB> boxex;
};

} // namespace Cubed