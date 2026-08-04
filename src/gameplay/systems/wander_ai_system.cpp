#include "Cubed/gameplay/systems/wander_ai_system.hpp"

#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/gameplay/ecs/server_entity.hpp"
#include "Cubed/tools/cubed_random.hpp"

namespace {
constexpr double DIRECTION_PROBABILITY = 0.01;
constexpr double MOVE_PROBABILITY = 0.01;
} // namespace

namespace Cubed {
void WanderAISystem::update(entt::registry& registry) {
    auto view =
        registry.view<AIBase, WanderAITag, BaseServerCreature, MoveBoost>();

    for (auto e : view) {
        auto [ai, creature, move_boost] =
            view.get<AIBase, BaseServerCreature, MoveBoost>(e);
        ++ai.count;
        if (ai.count >= ai.interval) {
            ai.count = 0;
            do_ai(creature, move_boost);
        }
    }
}

void WanderAISystem::do_ai(BaseServerCreature& creature,
                           MoveBoost& move_boost) {
    thread_local Random r{std::random_device()()};
    if (r.random_bool(DIRECTION_PROBABILITY)) {
        creature.transform.direction.value = r.random_direction_horizontal();
    }
    if (r.random_bool(MOVE_PROBABILITY)) {

        move_boost.duration = r.random_int(20, 40);
        move_boost.count = 0;
    }
}

} // namespace Cubed