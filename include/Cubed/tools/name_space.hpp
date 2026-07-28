#pragma once

#include <string_view>
#include <vector>
namespace Cubed {
inline std::vector<std::string_view> parse_namespace(std::string_view str) {
    std::vector<std::string_view> space;
    space.reserve(4);
    std::size_t p = str.find(':');
    std::size_t start = 0;
    while (p != std::string_view::npos) {
        space.emplace_back(str.substr(start, p));
        start = p + 1;
        p = str.find(':', p + 1);
    }
    space.emplace_back(str.substr(start));
    return space;
}
} // namespace Cubed