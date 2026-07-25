#pragma once
#include "Cubed/gameplay/block.hpp"

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
};
} // namespace Cubed