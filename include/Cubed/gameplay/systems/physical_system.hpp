#pragma once

#include "Cubed/gameplay/entity.hpp"
#include "Cubed/gameplay/world.hpp"
namespace Cubed {
class PhysicalSystem {
public:
    static glm::vec3 get_move_distance(float dt, const Entity& e);
    static std::tuple<bool, bool, bool> update(float dt, Entity& e,
                                               World& world);
    static std::tuple<bool, bool, bool>
    update(float dt, Entity& e, World& world, glm::vec3& moved_pos);

private:
};
} // namespace Cubed