#include "Cubed/gameplay/move_system.hpp"

#include "Cubed/gameplay/entity.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
namespace Cubed {
void MoveSystem::update(World& world, entt::registry& registry) {
    auto view = registry.view<Transform, Velocity, EntityInfo>();
    for (auto entity : view) {
        auto [transform, v, info] =
            view.get<Transform, Velocity, EntityInfo>(entity);
        move_x(world, transform, v, info);
        move_y(world, transform, v, info);
        move_z(world, transform, v, info);
    }
}

void MoveSystem::move_x(World& world, Transform& transform, Velocity& v,
                        const EntityInfo& info) {

    auto& pos = transform.pos;
    float distance = v.dx * world.get_per_tick_time() / 1000.0f;
    pos.x += distance;

    AABB box = HitboxManager::aabb(
        std::format("model/creature/{}/collision.json", info.name));
    glm::vec3 min = box.min();
    glm::vec3 max = box.max();

    int minx = std::floor(min.x);
    int maxx = std::floor(max.x);
    int miny = std::floor(min.y);
    int maxy = std::floor(max.y);
    int minz = std::floor(min.z);
    int maxz = std::floor(max.z);

    for (int x = minx; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            for (int z = minz; z <= maxz; ++z) {
                glm::ivec3 block_pos{x, y, z};
                if (!world.can_pass_block(block_pos)) {
                    AABB block_box = World::get_block_aabb(block_pos);
                    if (box.intersects(block_box)) {
                        pos.x -= distance;
                        v.dx = 0.0f;
                        return;
                    }
                }
            }
        }
    }
}

void MoveSystem::move_y(World& world, Transform& transform, Velocity& v,
                        const EntityInfo& info) {
    auto& pos = transform.pos;
    float distance = v.dy * world.get_per_tick_time() / 1000.0f;
    pos.y += distance;

    AABB box = HitboxManager::aabb(
        std::format("model/creature/{}/collision.json", info.name));
    glm::vec3 min = box.min();
    glm::vec3 max = box.max();

    int minx = std::floor(min.x);
    int maxx = std::floor(max.x);
    int miny = std::floor(min.y);
    int maxy = std::floor(max.y);
    int minz = std::floor(min.z);
    int maxz = std::floor(max.z);

    for (int x = minx; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            for (int z = minz; z <= maxz; ++z) {
                glm::ivec3 block_pos{x, y, z};
                if (!world.can_pass_block(block_pos)) {
                    AABB block_box = World::get_block_aabb(block_pos);
                    if (box.intersects(block_box)) {
                        pos.y -= distance;
                        v.dy = 0.0f;
                        return;
                    }
                }
            }
        }
    }
}

void MoveSystem::move_z(World& world, Transform& transform, Velocity& v,
                        const EntityInfo& info) {
    auto& pos = transform.pos;
    float distance = v.dz * world.get_per_tick_time() / 1000.0f;
    pos.z += distance;

    AABB box = HitboxManager::aabb(
        std::format("model/creature/{}/collision.json", info.name));
    glm::vec3 min = box.min();
    glm::vec3 max = box.max();

    int minx = std::floor(min.x);
    int maxx = std::floor(max.x);
    int miny = std::floor(min.y);
    int maxy = std::floor(max.y);
    int minz = std::floor(min.z);
    int maxz = std::floor(max.z);

    for (int x = minx; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            for (int z = minz; z <= maxz; ++z) {
                glm::ivec3 block_pos{x, y, z};
                if (!world.can_pass_block(block_pos)) {
                    AABB block_box = World::get_block_aabb(block_pos);
                    if (box.intersects(block_box)) {
                        pos.z -= distance;
                        v.dz = 0.0f;
                        return;
                    }
                }
            }
        }
    }
}

} // namespace Cubed