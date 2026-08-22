#pragma once
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/item.hpp"
namespace cubed {
struct ItemTag {
    ItemID id = 0;
    uint32_t count = 1;
};

struct PickupDelay {
    TickType remaining_ticks = 10;
};

} // namespace cubed
