#pragma once

#include <cstdint>
#include <string>
namespace Cubed {
struct ChatMessage {
    std::string player;
    std::string text;
    uint64_t time;
};
} // namespace Cubed