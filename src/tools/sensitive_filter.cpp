#include "Cubed/tools/sensitive_filter.hpp"

#include <queue>
#include <utf8cpp/utf8.h>
using nlohmann::json;
namespace Cubed {
SensitiveFilter::SensitiveFilter() {

};

void SensitiveFilter::load(const nlohmann::json& j) {
    trie.clear();
    trie.emplace_back();
    for (const auto& item : j["words"]) {

        auto str = item.get<std::string>();
        std::u32string word;
        utf8::utf8to32(str.begin(), str.end(), std::back_inserter(word));
        insert(word);
    }
    build();
}
std::string SensitiveFilter::filter(std::string_view str) {
    if (!utf8::is_valid(str.begin(), str.end())) {
        return {};
    }
    int state = 0;
    std::vector<char32_t> text;

    auto it = str.begin();

    while (it != str.end()) {
        text.push_back(utf8::next(it, str.end()));
    }
    std::vector<uint8_t> mask(text.size(), 0);
    for (size_t i = 0; i < text.size(); i++) {
        char32_t ch = text[i];

        auto it = trie[state].next.find(ch);

        while (state && it == trie[state].next.end()) {
            state = trie[state].fail;
            it = trie[state].next.find(ch);
        }

        if (it != trie[state].next.end()) {
            state = it->second;
        }

        int t = state;

        while (t) {
            if (trie[t].end) {
                size_t begin = i + 1 - trie[t].length;
                for (size_t j = begin; j < begin + trie[t].length; j++)
                    mask[j] = true;
            }

            t = trie[t].fail;
        }
    }
    std::string out;
    for (size_t i = 0; i < text.size();) {
        if (mask[i]) {
            out += "***";

            while (i < text.size() && mask[i])
                ++i;
        } else {
            // char32_t -> UTF8
            utf8::append(text[i], std::back_inserter(out));
            ++i;
        }
    }
    return out;
}

void SensitiveFilter::insert(const std::u32string& word) {
    int now = 0;
    for (char32_t ch : word) {
        auto it = trie[now].next.find(ch);
        if (it == trie[now].next.end()) {
            trie[now].next[ch] = trie.size();
            trie.emplace_back();
        }
        now = trie[now].next[ch];
    }
    trie[now].end = true;
    trie[now].length = word.size();
}

void SensitiveFilter::build() {
    std::queue<int> q;

    for (auto [ch, to] : trie[0].next) {
        trie[to].fail = 0;
        q.push(to);
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (auto [ch, v] : trie[u].next) {
            int f = trie[u].fail;

            while (f && !trie[f].next.count(ch))
                f = trie[f].fail;

            if (trie[f].next.count(ch))
                trie[v].fail = trie[f].next[ch];

            q.push(v);
        }
    }
}

} // namespace Cubed