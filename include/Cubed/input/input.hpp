#pragma once

#include <GLFW/glfw3.h>

namespace Cubed {

struct MoveState {
    bool forward = false;
    bool back = false;
    bool left = false;
    bool right = false;
    bool down = false;
    bool up = false;
};

struct MouseState {
    bool left = false;
    bool right = false;
};

} // namespace Cubed