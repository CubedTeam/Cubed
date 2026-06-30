#pragma once
#include <glm/glm.hpp>

namespace Cubed {

namespace Math {

void extract_frustum_planes(const glm::mat4& mvp_matrix,
                            std::vector<glm::vec4>& planes);

float smootherstep(float edge0, float edge1, float x);
bool is_aabb_in_frustum(const glm::vec3& center, const glm::vec3& half_extents,
                        const std::vector<glm::vec4>& planes);
float deterministic_random(int x, int z, uint64_t seed);
glm::vec3 slerp(const glm::vec3& from, const glm::vec3& to, float t);

inline float distance2(const glm::vec3& a, const glm::vec3& b) {
    glm::vec3 diff = a - b;
    return glm::dot(diff, diff);
}

} // namespace Math

} // namespace Cubed