#pragma once
#include "Cubed/tools/toml.utils.hpp"

namespace Cubed {

class Config {
public:
    explicit Config(std::string_view path);
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(const Config&) = delete;
    Config& operator=(Config&&) = delete;
    ~Config();

    toml::table& table();

    void load_config();
    void save_to_file();

    template <TOML::TomlValueType T>
    T get(std::string_view key, T default_value) {
        if (!m_init) {
            load_config();
        }
        if (auto* node = find_node(m_tbl, key)) {
            if (auto value = node->value<T>())
                return *value;
        }

        set(key, default_value);

        return default_value;
    }

    template <TOML::TomlValueType T> void set(std::string_view key, T&& value) {
        if (!m_init) {
            load_config();
        }
        auto pos = key.rfind('.');

        toml::table* table;
        std::string_view name;

        if (pos == std::string_view::npos) {
            table = &m_tbl;
            name = key;
        } else {
            table = find_or_create_table(key.substr(0, pos));
            name = key.substr(pos + 1);
        }
        // Insert node at the last level's table
        table->insert_or_assign(name, std::forward<T>(value));
    }

    template <typename T> void set_and_save(std::string_view key, T&& val) {
        set(key, std::forward(val));
        save_to_file();
    }

private:
    toml::table m_tbl;
    bool m_init = false;
    const std::string CONGIF_PATH;
    const toml::node* find_node(const toml::table& root,
                                std::string_view path) const;
    // Follow the path to find the last-level toml::table, creating it if it
    // does not exist
    toml::table* find_or_create_table(std::string_view path);
};

template <>
inline float Config::get(std::string_view key, float default_value) {
    return static_cast<float>(
        Config::get<double>(key, static_cast<double>(default_value)));
}

template <> inline void Config::set(std::string_view key, float&& value) {
    Config::set<double>(key, static_cast<double>(value));
}

} // namespace Cubed
