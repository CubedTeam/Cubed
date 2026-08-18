#pragma once
#include "Cubed/gameplay/item.hpp"
#include "Cubed/gameplay/item_manager.hpp"
namespace Cubed {

constexpr size_t HOTBAR_STACK_SUM = 10;
constexpr size_t INVENTORY_STACK_SUM = 30;

struct ItemStack {
    ItemID item = 0;
    size_t sum = 0;
    [[nodiscard]]
    uint32_t max_stack_size() const {
        return ItemManager::get(item).max_stack_size;
    }
};
} // namespace Cubed
