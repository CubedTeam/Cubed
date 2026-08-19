#pragma once
#include "Cubed/gameplay/ecs/movement.hpp"

#include <entt/entt.hpp>
namespace cubed {
class ServerWorld;
class PhysicalSystem {
public:
    static glm::vec3 get_move_distance(const TickVelocity& v);

    static void update(ServerWorld& world, entt::registry& registry,
                       entt::entity e);

private:
};
} // namespace cubed
