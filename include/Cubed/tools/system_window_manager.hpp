#pragma once

#include "Cubed/tools/log.hpp"

#include <cstdlib>
#include <string>
namespace Cubed {
enum class WindowManager { WINDOWS, UNKNOWN, NIRI, SWAY, HYPRLAND, KDE, GNOME };

namespace Tools {
inline WindowManager detect_wm() {
#ifdef _WIN32
    return WindowManager::WINDOWS;
#endif
#ifdef __linux__

    const char* desktop = getenv("XDG_CURRENT_DESKTOP");

    if (!desktop)
        return WindowManager::UNKNOWN;
    Logger::info("XDG_CURRENT_DESKTOP {}", desktop);
    std::string wm(desktop);

    if (wm.find("niri") != std::string::npos)
        return WindowManager::NIRI;

    if (wm.find("sway") != std::string::npos)
        return WindowManager::SWAY;

    if (wm.find("Hyprland") != std::string::npos)
        return WindowManager::HYPRLAND;

    if (wm.find("KDE") != std::string::npos)
        return WindowManager::KDE;

    if (wm.find("GNOME") != std::string::npos)
        return WindowManager::GNOME;

#endif

    return WindowManager::UNKNOWN;
}
} // namespace Tools

} // namespace Cubed
