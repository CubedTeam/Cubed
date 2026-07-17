#pragma once

#include "Cubed/tools/log.hpp"

#include <cstdlib>
#include <string>
namespace Cubed {

namespace Tools {
inline std::string detect_wm() {
#ifdef _WIN32
    return "Windows";
#endif
#ifdef __linux__

    const char* desktop = getenv("XDG_CURRENT_DESKTOP");
    if (!desktop) {
        return "Unknown";
    }
    Logger::info("XDG_CURRENT_DESKTOP: {}", desktop);
    return std::string(desktop);
#endif

    return "Unknown";
}
} // namespace Tools

} // namespace Cubed
