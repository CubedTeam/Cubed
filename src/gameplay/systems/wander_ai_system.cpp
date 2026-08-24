#include "Cubed/gameplay/systems/wander_ai_system.hpp"

#include "Cubed/gameplay/ecs/ai_struct.hpp"
#include "Cubed/tools/cubed_random.hpp"

namespace {
constexpr double DIRECTION_PROBABILITY = 0.01;
constexpr double MOVE_PROBABILITY = 0.01;
} // namespace

namespace cubed {
void WanderAISystem::update(entt::registry& registry, entt::entity e) {
    if (!registry.all_of<AIBase, WanderAITag, Transform, MoveBoost>(e)) {
        return;
    }

    auto [ai, transform, move_boost] =
        registry.get<AIBase, Transform, MoveBoost>(e);
    ++ai.count;
    if (ai.count >= ai.interval) {
        ai.count = 0;
        do_ai(transform, move_boost);
    }
}

void WanderAISystem::do_ai(Transform& transform, MoveBoost& move_boost) {
    thread_local Random r{std::random_device()()};
    if (r.random_bool(DIRECTION_PROBABILITY)) {
        transform.direction.value = r.random_direction_horizontal();
    }
    if (r.random_bool(MOVE_PROBABILITY)) {

        move_boost.duration = r.random_int(20, 40);
        move_boost.count = 0;
    }
}

} // namespace cubed
