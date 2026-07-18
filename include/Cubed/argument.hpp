#pragma once
#include <optional>
#include <string>
namespace Cubed {
struct Argument {
    std::optional<int> port;
    std::optional<std::string> ip;
    std::optional<std::string> player;
    std::optional<std::string> language;
    std::optional<std::string> video_driver;
    std::optional<bool> enable_exclusive;
    std::optional<std::string> logs_path;
    std::optional<int> log_level;
    std::optional<bool> enable_filelog;
    std::optional<bool> enable_consolelog;
};
} // namespace Cubed