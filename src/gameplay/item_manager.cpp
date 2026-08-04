#include "Cubed/gameplay/item_manager.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
namespace fs = std::filesystem;
using namespace rapidjson;
namespace Cubed {
ItemManager::ItemManager() {}

ItemManager& ItemManager::instance() {
    static ItemManager inst;
    return inst;
}

void ItemManager::init() {
    m_id_map.clear();
    m_map.clear();

    fs::path dir = ASSETS_PATH "item";
    if (!fs::is_directory(dir)) {
        throw std::runtime_error("Item path not exist!");
    }
    for (const auto& entry : fs::recursive_directory_iterator(
             dir, fs::directory_options::skip_permission_denied)) {
        if (fs::is_regular_file(entry)) {
            if (entry.path().extension().string() == ".json") {
                add(entry.path());
            }
        }
    }
}

void ItemManager::add(const std::filesystem::path& path) {

    std::ifstream s(path);
    if (!s.is_open()) {
        return;
    }
    IStreamWrapper isw(s);

    Document doc;
    doc.ParseStream(isw);
    if (doc.HasParseError()) {
        Logger::error("Can't Parse File {}, error code {}", path.string(),
                      static_cast<int>(doc.GetParseError()));
        return;
    }
    ItemData data;
    if (doc.HasMember("id")) {
        data.id = static_cast<ItemID>(doc["id"].GetInt());
    }
    if (doc.HasMember("name")) {
        data.name = doc["name"].GetString();
    }
    if (doc.HasMember("description")) {
        data.description = doc["description"].GetString();
    }
    if (doc.HasMember("texture")) {
        data.path = doc["texture"].GetString();
    }
    acc a;
    if (m_map.emplace(a, data.id, std::move(data))) {
        if (!m_id_map.emplace(a->second.name, a->first)) {
            Logger::error("ItemManager: Can't Insterd {} {} to id map",
                          a->second.name, a->first);
        }
    } else {
        Logger::error("ItemManager: Can't Insterd {} to map", path.string());
    }
}

const ItemData& ItemManager::get_item_data(std::string_view key) const {
    IDMap::const_accessor cacc;
    ItemID id = 0;
    if (m_id_map.find(cacc, key)) {
        id = cacc->second;
    } else {
        Logger::error("Can't Find key {} in id map", key);
        ASSERT(false);
        return EMPTY;
    }
    return get_item_data(id);
}
const ItemData& ItemManager::get_item_data(ItemID id) const {
    cacc c;
    if (!m_map.find(c, id)) {
        Logger::error("Can't find item {} in map", id);
        ASSERT(false);
        return EMPTY;
    }
    return c->second;
}

const ItemData& ItemManager::get(std::string_view key) {
    return instance().get_item_data(key);
}
const ItemData& ItemManager::get(ItemID id) {
    return instance().get_item_data(id);
}

} // namespace Cubed