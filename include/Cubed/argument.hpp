#pragma once
#include <optional>
#include <string>
namespace Cubed {
struct Argument {
    std::optional<int> port;
    std::optional<std::string> ip;
    std::optional<std::string> player;
    std::optional<bool> no_debug;
    std::optional<std::string> language;
    std::optional<std::string> video_driver;
    std::optional<bool> enable_exclusive;
};
} // namespace Cubed