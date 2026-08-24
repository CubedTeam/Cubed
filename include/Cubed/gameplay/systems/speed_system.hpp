#pragma once

#include <entt/entt.hpp>
namespace cubed {
class SpeedSystem {
public:
    static void update(float dt, entt::registry& registry, entt::entity e);

private:
    static void update_gravity(float dt, entt::registry& registry,
                               entt::entity e);
    static void update_horizontal_velocity(float dt, entt::registry& registry,
                                           entt::entity e);
};
} // namespace cubed
