#pragma once
#include "Cubed/gameplay/block.hpp"
#include "Cubed/gameplay/model.hpp"

#include <cstdint>
#include <string>
#include <variant>
namespace cubed {
using ItemID = uint16_t;

enum class ItemKind {
    NONE,
    BLOCK,
    SPAWN_EGG

};

using ItemProperty = std::variant<BlockType, ResourceLocation>;

struct ItemData {
    ItemID id = 0;
    ResourceLocation name;
    std::string local_name;
    std::string description;
    std::optional<ResourceLocation> texture_path;
    std::optional<ModelID> model_id;
    ItemKind kind = ItemKind::NONE;
    ItemProperty property;
    uint32_t max_stack_size = 64;
};

inline constexpr ItemKind get_item_kind(std::string_view kind) {
    if (kind == "block") {
        return ItemKind::BLOCK;
    }
    if (kind == "spawn_egg") {
        return ItemKind::SPAWN_EGG;
    }
    return ItemKind::NONE;
}

} // namespace cubed
