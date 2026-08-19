#pragma once

#include "Cubed/gameplay/game_time.hpp"
namespace cubed {
struct AIBase {
    TickType interval = 1;
    TickType count = 0;
};

struct WanderAITag {};

struct MoveBoost {
    TickType duration = 0;
    TickType count = 0;
};

} // namespace cubed
