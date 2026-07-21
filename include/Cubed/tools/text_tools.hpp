#pragma once
#include "Cubed/tools/cubed_assert.hpp"

#include <cstdint>
#include <string>
#include <utf8cpp/utf8.h>
#include <vector>
namespace Cubed {
inline std::string codepoint_to_utf8(uint32_t cp) {
    std::string out;

    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }

    return out;
}

inline void utf8_pop_back(std::string& str) {
    if (str.empty()) {
        return;
    }

    std::size_t i = str.size() - 1;

    while (i > 0 && (static_cast<unsigned char>(str[i]) & 0xC0) == 0x80) {
        --i;
    }

    str.erase(i);
}

inline std::vector<std::string> split_utf8(const std::string& str, int size) {
    ASSERT(size > 0);
    if (!utf8::is_valid(str)) {
        return {};
    }
    std::vector<std::string> blocks;
    auto cur = str.begin();
    int chat_count = 0;

    for (auto it = str.begin(); it != str.end();) {
        if (chat_count >= size) {
            blocks.emplace_back(cur, it);
            cur = it;
            chat_count = 0;
        }
        utf8::next(it, str.end());
        ++chat_count;
        if (it == str.end()) {
            blocks.emplace_back(cur, it);
        }
    }
    return blocks;
}

} // namespace Cubed