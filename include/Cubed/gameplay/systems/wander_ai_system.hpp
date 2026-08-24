#pragma once
#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"

#include <entt/entt.hpp>
namespace cubed {
class WanderAISystem {
public:
    static void update(entt::registry& registry, entt::entity e);

private:
    static void do_ai(Transform& transform, MoveBoost& move_boost);
};
} // namespace cubed
