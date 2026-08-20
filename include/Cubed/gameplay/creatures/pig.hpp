#pragma once

namespace cubed {
struct PigTag {};

namespace pig_defaults {
constexpr float MAX_SPEED = 0.1f;     // tiles/tick → 2 tiles/sec
constexpr float ACCELERATION = 0.01f; // ~10 ticks (0.5s) to reach full speed
constexpr float DECELERATION = 0.02f; // ~5 ticks (0.25s) to stop
constexpr float GRAVITY = 1.0f;       // ≈20 tiles/s², close to the player
} // namespace pig_defaults

} // namespace cubed
