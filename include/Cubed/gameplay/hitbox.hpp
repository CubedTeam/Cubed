#pragma once
#include <glm/glm.hpp>

namespace cubed {

using HitboxID = uint32_t;

struct Hitbox {
    glm::vec3 center{0.0f};
    glm::vec3 half{0.0f};

    Hitbox(glm::vec3 center_point, glm::vec3 half_size)
        : center(center_point), half(half_size) {}

    Hitbox() {};

    glm::vec3 min() const { return center - half; }

    glm::vec3 max() const { return center + half; }

    bool intersects(const Hitbox& other) const {
        return (glm::abs(center.x - other.center.x) <= half.x + other.half.x) &&
               (glm::abs(center.y - other.center.y) <= half.y + other.half.y) &&
               (glm::abs(center.z - other.center.z) <= half.z + other.half.z);
    }
};

} // namespace cubed
