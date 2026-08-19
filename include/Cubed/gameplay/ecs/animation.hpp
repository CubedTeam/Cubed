#pragma once

#include "Cubed/gameplay/gait.hpp"
namespace cubed {

struct WalkPose {
    Gait gait = Gait::STOP;
    // for arm roll caculate
    float walk_time = 0.0f;
    // for sound play
    float moving_time = 0.0f;
};

} // namespace cubed
