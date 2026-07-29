#pragma once
#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/state.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/world.hpp"
namespace Cubed {

class PhysicalSystem {
public:
    static glm::vec3 get_move_distance(float dt, const Direction& d,
                                       const Velocity& v);

    static std::tuple<bool, bool, bool>
    update(float dt, World& world, glm::vec3& moved_pos, Velocity& v,
           Direction& direction, MoveState& move_state, HitboxID hitbox);

private:
};
} // namespace Cubed