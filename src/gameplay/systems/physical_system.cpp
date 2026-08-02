#include "Cubed/gameplay/systems/physical_system.hpp"

#include "Cubed/gameplay/ecs/server_entity.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/gameplay/server_world.hpp"
namespace Cubed {

namespace {

bool update_x(const glm::vec3& pos, const glm::vec3& distance,
              ServerWorld& world, const Hitbox& box) {
    glm::vec3 p = pos;
    p.x += distance.x;
    Hitbox b = box;
    b.center += p;
    glm::vec3 min = b.min();
    glm::vec3 max = b.max();
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
                    Hitbox block_box = World::get_block_aabb(block_pos);
                    if (b.intersects(block_box)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool update_y(const glm::vec3& pos, const glm::vec3& distance,
              ServerWorld& world, const Hitbox& box) {
    glm::vec3 p = pos;
    p.y += distance.y;
    Hitbox b = box;
    b.center += p;
    glm::vec3 min = b.min();
    glm::vec3 max = b.max();
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
                    Hitbox block_box = World::get_block_aabb(block_pos);
                    if (b.intersects(block_box)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool update_z(const glm::vec3& pos, const glm::vec3& distance,
              ServerWorld& world, const Hitbox& box) {
    glm::vec3 p = pos;
    p.z += distance.z;
    Hitbox b = box;
    b.center += p;
    glm::vec3 min = b.min();
    glm::vec3 max = b.max();
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
                    Hitbox block_box = World::get_block_aabb(block_pos);
                    if (b.intersects(block_box)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

} // namespace

void PhysicalSystem::update(ServerWorld& world, entt::registry& registry) {

    auto view = registry.view<BaseServerCreature>();
    for (auto e : view) {
        auto& creature = view.get<BaseServerCreature>(e);
        auto distance = get_move_distance(creature.velocity);
        auto box = HitboxManager::hitbox(creature.hitbox);
        auto& pos = creature.transform.position.value;
        auto& v = creature.velocity;
        if (update_x(pos, distance, world, box.box)) {
            pos.x += distance.x;

        } else {
            v.value.x = 0.0f;
        }

        if (update_y(pos, distance, world, box.box)) {
            pos.y += distance.y;

        } else {
            v.value.y = 0.0f;
        }

        if (update_z(pos, distance, world, box.box)) {
            pos.z += distance.z;

        } else {
            v.value.z = 0.0f;
        }
    }
}

glm::vec3 PhysicalSystem::get_move_distance(const TickVelocity& v) {
    return v.value; // Per-tick forward distance equals the velocity magnitud
}
} // namespace Cubed