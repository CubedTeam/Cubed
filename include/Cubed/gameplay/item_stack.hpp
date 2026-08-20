#pragma once
#include "Cubed/gameplay/item.hpp"
#include "Cubed/gameplay/item_manager.hpp"
namespace cubed {

constexpr size_t HOTBAR_SIZE = 10;
constexpr size_t BACKPACK_SIZE = 30;
constexpr std::size_t INVENTORY_SIZE = HOTBAR_SIZE + BACKPACK_SIZE;

struct StoredItemStack {
    ItemID item_id = 0;
    size_t count = 0;
    size_t position = 0;
};

struct ItemStack {
    ItemID item = 0;
    size_t count = 0;

    [[nodiscard]]
    uint32_t max_stack_size() const {
        return ItemManager::get(item).max_stack_size;
    }
    bool operator==(const ItemStack&) const = default;
};
} // namespace cubed
