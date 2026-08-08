#pragma once

#include <SDL3/SDL_timer.h>
#include <chrono>
#include <cstdint>
#include <format>
#include <string>
namespace Cubed {
namespace Tools {
// return ms
inline uint64_t get_time_ticks() { return SDL_GetTicks(); }

inline std::string get_time_date_str() {
    static auto* zone = std::chrono::current_zone();
    auto now = std::chrono::system_clock::now();
    auto local =
        std::chrono::time_point_cast<std::chrono::seconds>(zone->to_local(now));
    return std::format("{:%Y-%m-%d_%H-%M-%S}", local);
}

} // namespace Tools

} // namespace Cubed