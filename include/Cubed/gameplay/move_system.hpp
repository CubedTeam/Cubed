#pragma once
#include "Cubed/gameplay/entity.hpp"
#include "Cubed/gameplay/world.hpp"

#include <entt/entt.hpp>
namespace Cubed {

class MoveSystem {
public:
    static void update(World& world, entt::registry& registry);

private:
    static void move_x(World& world, Transform& transform, Velocity& v,
                       const EntityInfo& info);
    static void move_y(World& world, Transform& transform, Velocity& v,
                       const EntityInfo& info);
    static void move_z(World& world, Transform& transform, Velocity& v,
                       const EntityInfo& info);
};
} // namespace Cubed