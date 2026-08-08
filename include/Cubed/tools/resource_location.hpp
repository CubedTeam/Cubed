#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
namespace Cubed {
struct ResourceLocation {
    static constexpr std::string_view DEFAULT_NAMESPACE = "cubed";
    std::string ns = std::string(DEFAULT_NAMESPACE);
    std::string path;
    std::string to_string() const { return ns + ":" + path; }

    // Parses "ns:path"; ns defaults to "cubed" when no colon present.
    static std::optional<ResourceLocation> parse(std::string_view str) {
        std::regex pattern(R"([a-zA-Z0-9._:/-]+)");
        if (!std::regex_match(str.begin(), str.end(), pattern)) {
            return std::nullopt;
        }
        auto it = str.find(":");
        if (it == std::string_view::npos) {
            return ResourceLocation{std::string(DEFAULT_NAMESPACE),
                                    std::string(str)};
        }
        return ResourceLocation{std::string(str.substr(0, it)),
                                std::string(str.substr(it + 1))};
    }

    bool operator==(const ResourceLocation& o) const {
        return (ns == o.ns) && (path == o.path);
    }
    std::size_t hash() const { return std::hash<std::string>()(to_string()); }
};
} // namespace Cubed