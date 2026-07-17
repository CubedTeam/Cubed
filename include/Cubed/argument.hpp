#pragma once
#include <optional>
#include <string>
namespace Cubed {
struct Argument {
    std::optional<int> port;
    std::optional<std::string> ip;
    std::optional<std::string> player;
    std::optional<bool> debug_on;
    std::optional<std::string> language;
};
} // namespace Cubed