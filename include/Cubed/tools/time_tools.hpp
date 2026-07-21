#pragma once

#include <SDL3/SDL_timer.h>
#include <cstdint>
namespace Cubed {
namespace Tools {
inline uint64_t get_time_ticks() { return SDL_GetTicks(); }
} // namespace Tools

} // namespace Cubed