#pragma once

#include <glm/glm.hpp>

namespace Cubed {
constexpr glm::vec3 SUN_COLOR{1.00f, 0.95f, 0.80f};
constexpr glm::vec3 MOON_COLOR{0.75f, 0.80f, 1.00f};

constexpr glm::vec3 SUNSET_SUNLIGHT_COLOR{1.00f, 0.45f, 0.15f};
constexpr glm::vec3 NOON_SUNLIGHT_COLOR{1.00f, 0.90f, 0.65f};
constexpr glm::vec3 SUNSET_AMBIENT_COLOR{0.18f, 0.12f, 0.35f};
constexpr glm::vec3 NOON_AMBIENT_COLOR{0.35f, 0.50f, 0.85f};
constexpr glm::vec3 MOONLIGHT_COLOR{0.55f, 0.70f, 1.00f};
constexpr glm::vec3 NIGHT_AMBIENT_COLOR{0.08f, 0.10f, 0.18f};
constexpr float FAR_PLANE = 1000.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float SUN_SIZE = 50.0f;
constexpr float MOON_SIZE = 50.0f;
constexpr float DEPTH_MAP_SIZE = 4096.0f;
constexpr float ANGLE_STEP_DEG = 0.5f;
} // namespace Cubed