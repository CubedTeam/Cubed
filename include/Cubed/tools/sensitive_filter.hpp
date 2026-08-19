#pragma once
#include <rapidjson/document.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace cubed {
class SensitiveFilter {
public:
    SensitiveFilter();
    void load(const rapidjson::Document& doc);
    std::string filter(std::string_view text);

private:
    void insert(const std::u32string& world);
    void build();

    struct Node {
        std::unordered_map<char32_t, int> next;
        int fail = 0;
        size_t length = 0;
        bool end = false;
    };

    std::vector<Node> trie{1};
};
} // namespace cubed
