#pragma once

#include "Cubed/ui/color.hpp"

#include <cstdint>
#include <string>
namespace cubed {
struct ChatMessage {
    std::string player;
    std::string text;
    Color color;
    bool system_msg = false;
    uint64_t time = 0;
};
} // namespace cubed
