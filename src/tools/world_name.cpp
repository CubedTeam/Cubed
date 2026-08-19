#include "Cubed/tools/world_name.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <utf8cpp/utf8.h>

namespace cubed::tools {
bool is_valid_world_name(std::string_view world_name) {
    if (world_name.empty() || world_name == "." || world_name == ".." ||
        !utf8::is_valid(world_name.begin(), world_name.end())) {
        return false;
    }

    // AI-generated: Keep save names portable across supported platforms.
    constexpr std::string_view INVALID_CHARS = "<>:\"/\\|?*";
    for (const unsigned char CH : world_name) {
        if (CH < 0x20 || INVALID_CHARS.contains(static_cast<char>(CH))) {
            return false;
        }
    }

    if (world_name.back() == ' ' || world_name.back() == '.') {
        return false;
    }

    std::string base(world_name.substr(0, world_name.find('.')));
    std::ranges::transform(base, base.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });

    constexpr std::array RESERVED_NAMES = {
        "CON",  "PRN",  "AUX",  "NUL",  "COM1", "COM2", "COM3", "COM4",
        "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3",
        "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
    };
    return std::ranges::find(RESERVED_NAMES, base) == RESERVED_NAMES.end();
}
} // namespace cubed::tools
