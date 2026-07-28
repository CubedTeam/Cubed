#pragma once

#include "Cubed/gameplay/ecs/server_entity.hpp"
#include "Cubed/gameplay/ecs/state.hpp"

namespace Cubed {
class SpeedSystem {
public:
    static void update(float dt, ServerEntity& e);
    static void update(float dt, Velocity& v, MoveState& move_state,
                       Movement& movement, Direction& direction,
                       const Gravity& g);

private:
};
} // namespace Cubed