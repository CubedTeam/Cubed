#pragma once

#include "Cubed/tools/resource_location.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

namespace cubed {

using BlockType = uint8_t;
using OptionalBlockVectorArray =
    std::array<std::optional<std::vector<BlockType>>, 4>;

struct BlockTexture {
    std::string name;
    unsigned id;
    std::vector<GLuint> texture;
};

struct Block : public BlockTexture {};

struct BlockRenderData {
    std::vector<bool> draw_face;
    unsigned block_id = 0;
    BlockRenderData() = default;
    BlockRenderData(const BlockRenderData&) = default;
    BlockRenderData& operator=(const BlockRenderData&) = default;
    BlockRenderData(BlockRenderData&& data)
        : draw_face(std::move(data.draw_face)), block_id(data.block_id) {}
    BlockRenderData& operator=(BlockRenderData&& data) {
        draw_face = std::move(data.draw_face);
        block_id = data.block_id;
        return *this;
    }
};

struct LookBlock {
    glm::ivec3 pos;
    glm::ivec3 normal;
};

enum class BlockTextureType {
    NONE,
    CUBOID,
    CROSS

};

struct BlockSound {
    std::optional<ResourceLocation> break_s;
    std::optional<ResourceLocation> place;
    std::optional<ResourceLocation> walk;
};

struct BlockData {
    ResourceLocation name{};
    BlockType id = 0;

    bool is_liquid = false;
    bool is_gas = false;

    bool is_passable = false;
    bool is_cross_plane = false;
    bool is_transparent = false;

    bool is_discard = false;
    bool is_blend = false;
    bool is_transitional = false;
    float roughness = 1.0f;

    BlockTextureType texture_type = BlockTextureType::NONE;

    std::optional<ResourceLocation> texture_path;
    std::optional<ResourceLocation> normal;

    BlockSound sound;

    BlockData() = default;
};

} // namespace cubed
