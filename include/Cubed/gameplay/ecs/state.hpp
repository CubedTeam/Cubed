#pragma once

namespace cubed {
struct MoveState {

    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;

    bool down = false;
    bool up = false;

    bool is_fly = false;
    bool can_up = true;
};

} // namespace cubed
