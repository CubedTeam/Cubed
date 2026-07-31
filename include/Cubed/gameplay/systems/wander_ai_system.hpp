#pragma once
#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/gameplay/ecs/server_entity.hpp"

#include <entt/entt.hpp>
namespace Cubed {
class WanderAISystem {
public:
    static void update(entt::registry& registry);

private:
    static void do_ai(BaseServerCreature& creature, MoveBoost& move_boost);
};
} // namespace Cubed