#pragma once
#include <glm/glm.hpp>
namespace Cubed {
struct Position {
    glm::vec3 value{0.0f};
};

struct Orientation {
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
};

struct Direction {
    glm::vec3 value{0.0f};
};

struct Transform {
    Position position;
    Orientation orientation;
    Direction direction;
};

} // namespace Cubed