#pragma once
#include "Cubed/constants.hpp"

#include <glm/glm.hpp>
namespace Cubed {
struct TickVelocity {

    glm::vec3 value{0.0f};
    // blocks/tick!!! -1 for in
    glm::vec3 max{1.0f, -1.0f, 1.0f};
};

struct Velocity {

    glm::vec3 value{0.0f};
    // blocks/second!!!
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