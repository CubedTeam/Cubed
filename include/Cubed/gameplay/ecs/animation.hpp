#pragma once

#include "Cubed/gameplay/player.hpp"
namespace Cubed {

struct WalkPose {
    Gait gait = Gait::STOP;
    // for arm roll caculate
    float walk_time = 0.0f;
    // for sound play
    float moving_time = 0.0f;
};

} // namespace Cubed