#pragma once
#include "glm/ext/vector_float3.hpp"

#include <entt/entt.hpp>
#include <span>

namespace cubed {
class ServerPlayer;
class ServerEntityManager;
class ItemPickupSystem {
public:
    static void
    update(std::span<std::pair<const glm::vec3, std::shared_ptr<ServerPlayer>>>
               players,
           ServerEntityManager& manager, entt::registry& registry,
           entt::entity e);
};
} // namespace cubed