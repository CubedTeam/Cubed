#pragma once
#include "Cubed/constants.hpp"

#include <glm/glm.hpp>
namespace Cubed {
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
} // namespace Cubed