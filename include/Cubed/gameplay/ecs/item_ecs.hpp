#pragma once
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/item.hpp"
namespace cubed {
struct ItemTag {
    ItemID id = 0;
};

struct PickupDelay {
    TickType remaining_ticks = 10;
};

} // namespace cubed
