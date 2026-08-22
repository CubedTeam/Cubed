#include "Cubed/gameplay/systems/item_pickup_system.hpp"

#include "Cubed/gameplay/ecs/item_ecs.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/server/server_entity_manager.hpp"
#include "Cubed/gameplay/server/server_player.hpp"
#include "Cubed/tools/math_tools.hpp"

#include <algorithm>
namespace cubed {

void ItemPickupSystem::update(
    std::span<std::pair<const glm::vec3, std::shared_ptr<ServerPlayer>>>
        players,
    ServerEntityManager& manager, entt::registry& registry, entt::entity e) {
    if (!registry.all_of<ItemTag, PickupDelay, Transform, Entity>(e)) {
        return;
    }
    if (players.empty()) {
        return;
    }
    auto& delay = registry.get<PickupDelay>(e);
    if (delay.remaining_ticks > 0) {
        --delay.remaining_ticks;
        return;
    }
    const auto& transform = registry.get<Transform>(e);
    std::vector<std::pair<float, ServerPlayer*>> sort_player;

    for (auto& [pos, player] : players) {
        float distance2 = math::distance2(transform.position.value, pos);
        sort_player.emplace_back(distance2, player.get());
    }

    std::ranges::sort(sort_player,
                      [](const std::pair<float, ServerPlayer*>& a,
                         const std::pair<float, ServerPlayer*>& b) {
                          return a.first < b.first;
                      });

    const auto& item_tag = registry.get<ItemTag>(e);
    bool added = false;
    for (auto& [distance2, player] : sort_player) {
        if (!player) {
            continue;
        }
        if (distance2 > 2.25f) {
            continue;
        }
        if (player->atomic_add_item(item_tag.id)) {
            added = true;
            break;
        }
    }
    const auto& entity = registry.get<Entity>(e);

    if (added) {
        manager.destroy(entity.id);
    }
}
} // namespace cubed