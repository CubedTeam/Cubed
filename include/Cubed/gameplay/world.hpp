#pragma once
#include "Cubed/gameplay/block.hpp"
#include "Cubed/gameplay/hitbox.hpp"

#include <glm/glm.hpp>
namespace Cubed {
class World {
public:
    World() = default;
    World(const World&) = delete;
    World(World&&) = delete;
    World& operator=(const World&) = delete;
    World& operator=(World&&) = delete;
    virtual ~World() = default;

    virtual int get_block(const glm::ivec3& block_pos) const = 0;
    virtual bool is_solid(const glm::ivec3& block_pos) const = 0;
    virtual bool can_pass_block(const glm::ivec3& block_pos) const = 0;
    virtual BlockType get_block_tpye(const glm::ivec3& block_pos) const = 0;
    virtual int get_per_tick_time() const = 0;

    static Hitbox get_block_aabb(const glm::ivec3& pos) {
        return {glm::vec3{static_cast<float>(pos.x) + 0.5f,
                          static_cast<float>(pos.y) + 0.5f,
                          static_cast<float>(pos.z) + 0.5f},
                glm::vec3{0.5f, 0.5f, 0.5f}};
    }
};

enum class RunMode { CLIENT_ONLY, SERVER_ONLY, HYBRID };

} // namespace Cubed