#include "Cubed/config.hpp"

#include "Cubed/tools/log.hpp"

#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

using namespace std::string_view_literals;

namespace Cubed {

Config::Config() { load_config(); }

Config::~Config() { save_to_file(); }

toml::table& Config::table() { return m_tbl; }

void Config::load_config() {
    fs::path config_path{CONGIF_PATH};

    if (fs::is_regular_file(config_path)) {
        try {
            m_tbl = toml::parse_file(config_path.string());
            Logger::info("Load Config File Success");
        } catch (const toml::parse_error& err) {
            Logger::error("Load Config Error: \"{}\"", err.what());
        }
    }
}
void Config::save_to_file() {
    fs::path config_path{CONGIF_PATH};
    std::ofstream file{config_path};
    file << m_tbl;
    Logger::info("Save File Success");
}

const toml::node* Config::find_node(const toml::table& root,
                                    std::string_view path) const {
    const toml::table* table = &root;

    size_t cur = 0;
    auto pos = path.find('.');

    while (pos != std::string_view::npos) {
        auto name = path.substr(cur, pos - cur);

        if (auto* next = (*table)[name].as_table()) {
            table = next;
        } else {
            return nullptr;
        }

        cur = pos + 1;
        pos = path.find('.', cur);
    }

    auto key = path.substr(cur);

    return (*table)[key].node();
}

toml::table* Config::find_or_create_table(std::string_view path) {
    toml::table* table = &m_tbl;

    while (true) {
        auto pos = path.find('.');
        auto part = pos == std::string_view::npos ? path : path.substr(0, pos);

        if (auto* next = (*table)[part].as_table()) {
            // If there is a table, proceed to the next
            table = next;
        } else {
            // If there is no table, create a new one
            auto [it, _] = table->insert(part, toml::table{});
            table = it->second.as_table();
        }

        if (pos == std::string_view::npos)
            break;

        path.remove_prefix(pos + 1);
    }

    return table;
}

} // namespace Cubed