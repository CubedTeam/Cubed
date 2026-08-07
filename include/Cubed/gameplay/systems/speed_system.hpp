#pragma once

#include <entt/entt.hpp>
namespace Cubed {
class SpeedSystem {
public:
    static void update(float dt, entt::registry& registry, entt::entity e);

private:
};
} // namespace Cubed