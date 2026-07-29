#pragma once

#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/state.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"

namespace Cubed {
class SpeedSystem {
public:
    static void update(float dt, Velocity& v, MoveState& move_state,
                       Movement& movement, Direction& direction,
                       const Gravity& g);

private:
};
} // namespace Cubed