#include "Cubed/gameplay/systems/physical_system.hpp"

#include "Cubed/gameplay/hitbox_manager.hpp"
namespace Cubed {

namespace {

bool update_x(const glm::vec3& pos, const glm::vec3& distance, World& world,
              const Hitbox& box) {
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

bool update_y(const glm::vec3& pos, const glm::vec3& distance, World& world,
              const Hitbox& box) {
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

bool update_z(const glm::vec3& pos, const glm::vec3& distance, World& world,
              const Hitbox& box) {
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

std::tuple<bool, bool, bool>
PhysicalSystem::update(float dt, World& world, glm::vec3& moved_pos,
                       Velocity& v, Direction& direction, MoveState& move_state,
                       HitboxID hitbox) {
    auto distance = get_move_distance(dt, direction, v);
    auto box = HitboxManager::hitbox(hitbox);
    bool x = false;
    bool y = false;
    bool z = false;
    if (update_x(moved_pos, distance, world, box.box)) {
        moved_pos.x += distance.x;
        x = true;
    } else {
        v.value.x = 0.0f;
    }

    if (update_y(moved_pos, distance, world, box.box)) {
        moved_pos.y += distance.y;
        y = true;
    } else {
        v.value.y = 0.0f;
        if (distance.y < 0) {
            move_state.can_up = true;
            move_state.is_fly = false;
        }
    }

    if (update_z(moved_pos, distance, world, box.box)) {
        moved_pos.z += distance.z;
        z = true;
    } else {
        v.value.z = 0.0f;
    }
    return {x, y, z};
}

glm::vec3 PhysicalSystem::get_move_distance(float dt, const Direction& d,
                                            const Velocity& v) {
    return glm::vec3{d.value.x * v.value.x * dt, v.value.y * dt,
                     d.value.z * v.value.z * dt};
}
} // namespace Cubed