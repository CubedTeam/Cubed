#pragma once
#include "Cubed/tools/log.hpp"

#include <filesystem>
#include <stdexcept>
#include <string>
#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on
#elif __linux__
#include <cstdlib>
#endif

namespace cubed::tools {

inline void open_file_manager(const std::filesystem::path& path) {
#ifdef _WIN32
    auto abs_path = std::filesystem::absolute(path);
    if (!std::filesystem::exists(abs_path)) {
        Logger::warn("Path does not exist: {}", abs_path.string());
        return;
    }
    HINSTANCE result =
        ShellExecuteW(nullptr, L"open", abs_path.wstring().c_str(), nullptr,
                      nullptr, SW_SHOWNORMAL);
    if ((INT_PTR)result <= 32) {
        Logger::warn("ShellExecuteW failed for path: {}, error code: {}",
                     path.string(), (int)(INT_PTR)result);
    }

#elif __linux__

    constexpr const char* FILE_MANAGERS[] = {"nautilus", "dolphin", "thunar",
                                             "nemo", "pcmanfm"};

    auto is_available = [](const char* exe) {
        std::string cmd = std::string("command -v ") + exe + " >/dev/null 2>&1";
        return std::system(cmd.c_str()) == 0;
    };

    const std::string QUOTED = "\"" + path.string() + "\"";
    for (const char* fm : FILE_MANAGERS) {
        if (!is_available(fm)) {
            continue;
        }
        std::string cmd = std::string(fm) + " " + QUOTED + " >/dev/null 2>&1 &";
        if (std::system(cmd.c_str()) == 0) {
            return;
        }
    }

    // Fallback: let the system choose a handler.
    if (is_available("xdg-open")) {
        std::string cmd = "xdg-open " + QUOTED + " >/dev/null 2>&1 &";
        if (std::system(cmd.c_str()) == 0) {
            return;
        }
    }

    Logger::warn("Failed to open file manager for {}", path.string());
#endif
}

inline void atomic_replace(const std::filesystem::path& temp_path,
                           const std::filesystem::path& target_path) {
#ifdef _WIN32
    if (!MoveFileExW(temp_path.c_str(), target_path.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        throw std::runtime_error("Failed to replace file, Win32 error: " +
                                 std::to_string(GetLastError()));
    }
#else
    std::error_code ec;
    std::filesystem::rename(temp_path, target_path, ec);

    if (ec) {
        throw std::runtime_error("Failed to replace file: " + ec.message());
    }
#endif
}

} // namespace cubed::tools
