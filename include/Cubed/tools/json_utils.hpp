#pragma once
#include "Cubed/tools/log.hpp"

#include <filesystem>
#include <rapidjson/document.h>
#include <string>
#include <type_traits>
#include <unordered_map>
namespace Cubed::Tools {

namespace detail {
template <typename T> inline constexpr bool always_false_v = false; // NOLINT
} // namespace detail

std::unordered_map<std::string, std::string>
doc_to_map(const rapidjson::Document& doc);

bool parse_json(rapidjson::Document& doc, const std::filesystem::path& path);

template <typename T>
bool get_json_value(const rapidjson::Value& value, const char* key, T& out) {
    using ValueType = std::decay_t<T>;
    if (!value.HasMember(key)) {
        Logger::error("json don't has key {}", key);
        return false;
    }
    const auto& v = value[key];
    if constexpr (std::is_same_v<ValueType, bool>) {
        if (!v.IsBool()) {
            Logger::error("json key {} value is not bool", key);
            return false;
        }
        out = v.GetBool();
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        if (!v.IsString()) {
            Logger::error("json key {} value is not string", key);
            return false;
        }
        out = v.GetString();
    } else if constexpr (std::is_same_v<ValueType, float>) {
        if (!v.IsNumber()) {
            Logger::error("json key {} value is not number", key);
            return false;
        }
        out = v.GetFloat();
    } else if constexpr (std::is_same_v<ValueType, double>) {
        if (!v.IsNumber()) {
            Logger::error("json key {} value is not number", key);
            return false;
        }
        out = v.GetDouble();
    } else if constexpr (std::is_integral_v<ValueType> &&
                         std::is_unsigned_v<ValueType>) {
        if (!v.IsUint64()) {
            Logger::error("json key {} value is not uint", key);
            return false;
        }
        out = static_cast<ValueType>(v.GetUint64());
    } else if constexpr (std::is_integral_v<ValueType> &&
                         std::is_signed_v<ValueType>) {
        if (!v.IsInt64()) {
            Logger::error("json key {} value is not int", key);
            return false;
        }
        out = static_cast<ValueType>(v.GetInt64());
    } else {
        static_assert(detail::always_false_v<ValueType>,
                      "get_json_value: unsupported type");
    }
    return true;
}
} // namespace Cubed::Tools