#pragma once

#include <format>
#include <string>
#include <string_view>
#include <unordered_map>
namespace Cubed {

struct FormatArg {
    std::string name;
    std::string value;
};
template <typename T> FormatArg arg(std::string_view name, T&& value) {
    return {std::string(name), std::format("{}", std::forward<T>(value))};
}

class Localization {
public:
    Localization();
    static Localization& instance();

    void load_language(std::string_view language);
    bool has(const std::string& key) const;
    template <typename... Args>
    std::string translate(const std::string& key, Args&&... args) {
        if constexpr (sizeof...(Args) == 0)
            return std::string{lookup(key)};
        std::string fmt{lookup(key)};
        (replace_all(fmt, "{" + args.name + "}", args.value), ...);
        return fmt;
    }

private:
    std::unordered_map<std::string, std::string> m_translations;
    std::string m_current_lang{"en_US"};
    std::string_view lookup(const std::string& key) const;
    static void replace_all(std::string& str, std::string_view from,
                            std::string_view to);
};

template <typename... Args>
std::string tr(const std::string& key, Args&&... args) {
    return Localization::instance().translate(key, std::forward<Args>(args)...);
}
template <typename T>
FormatArg tr_arg(const std::string& name, const std::string& key) {
    return {std::string(name), tr(key)};
}
} // namespace Cubed