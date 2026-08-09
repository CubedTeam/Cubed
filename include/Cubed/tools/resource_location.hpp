#pragma once

#include <cstddef>
#include <filesystem>
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
        if (str.empty() || str.contains("..") || str.front() == '/' ||
            str.front() == ':' || str.back() == ':') {
            return std::nullopt;
        }

        std::regex pattern(R"([a-zA-Z0-9._:/-]+)");
        if (!std::regex_match(str.begin(), str.end(), pattern)) {
            return std::nullopt;
        }

        auto it = str.find(':');
        std::string_view ns = DEFAULT_NAMESPACE;
        std::string_view path = str;
        if (it != std::string_view::npos) {
            if (str.find(':', it + 1) != std::string_view::npos) {
                return std::nullopt; // only one colon allowed
            }
            ns = str.substr(0, it);
            path = str.substr(it + 1);
            if (ns.empty() || path.empty()) {
                return std::nullopt;
            }
        }

        return ResourceLocation{std::string(ns), std::string(path)};
    }

    static std::filesystem::path get_assets_path_prefix(std::string_view ns) {
        return ASSETS_PATH + std::string(ns);
    }

    bool operator==(const ResourceLocation& o) const {
        return (ns == o.ns) && (path == o.path);
    }
    struct Hash {
        std::size_t hash(const ResourceLocation& p) const {
            return ResourceLocation::hash(p);
        }
        bool equal(const ResourceLocation& a, const ResourceLocation& b) const {
            return a == b;
        }
    };

    static std::size_t hash(const ResourceLocation& p) {
        return std::hash<std::string>()(p.to_string());
    }

    std::filesystem::path assets_path_prefix() const {
        return get_assets_path_prefix(ns);
    }

    std::filesystem::path full_path() const {
        auto prefix = assets_path_prefix();
        auto p = prefix / path;
        p = p.lexically_normal();
        auto root = prefix.lexically_normal();
        if (p.string().rfind(root.string(), 0) != 0 && p != root) {
            return {};
        }
        return p;
    }
};

} // namespace Cubed