#pragma once
#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"

#include <entt/entt.hpp>
namespace Cubed {
class ServerWorld;
class PhysicalSystem {
public:
    static glm::vec3 get_move_distance(const Direction& d,
                                       const TickVelocity& v);

    static void update(ServerWorld& world, entt::registry& registry);

private:
};
} // namespace Cubed