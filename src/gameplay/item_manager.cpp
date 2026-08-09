#include "Cubed/gameplay/item_manager.hpp"

#include "Cubed/gameplay/block_manager.hpp"
#include "Cubed/localization.hpp"
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

    fs::path root_path = ResourceLocation::get_assets_path_prefix(
        ResourceLocation::DEFAULT_NAMESPACE);
    fs::path item_path = root_path / "items";
    fs::create_directories(item_path);
    for (const auto& entry : fs::recursive_directory_iterator(
             item_path, fs::directory_options::skip_permission_denied)) {
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
        data.path = ResourceLocation::parse(doc["texture"].GetString());
    }
    if (doc.HasMember("type")) {
        std::string type = doc["type"].GetString();
        data.kind = get_item_kind(type);
    }
    if (data.kind == ItemKind::SPAWN_EGG) {
        if (doc.HasMember("creature")) {
            data.property = doc["creature"].GetString();
        } else {
            data.property = "cubed:pig";
        }
    }
    data.local_name = tr(std::format("item.{}.name", data.name));
    acc a;
    if (m_map.emplace(a, data.id, std::move(data))) {
        if (!m_id_map.emplace(a->second.name, a->first)) {
            Logger::error("ItemManager: Can't Insterd {} {} to id map",
                          a->second.name, a->first);
        }
        if (a->second.kind == ItemKind::BLOCK) {
            BlockType b = BlockManager::id_from_name(a->second.name);
            m_block_to_id_map.emplace(b, a->first);
            a->second.property = b;
        }

    } else {
        Logger::error("ItemManager: Can't Insterd {} to map", path.string());
    }
}

ItemData ItemManager::get_item_data(std::string_view key) const {
    ItemID id = 0;
    {
        IDMap::const_accessor cacc;

        if (m_id_map.find(cacc, std::string(key))) {
            id = cacc->second;
        } else {
            Logger::error("Can't Find key {} in id map", key);
            ASSERT(false);
            return EMPTY;
        }
    }

    return get_item_data(id);
}
ItemData ItemManager::get_item_data(ItemID id) const {
    cacc c;
    if (!m_map.find(c, id)) {
        Logger::error("Can't find item {} in map", id);
        ASSERT(false);
        return EMPTY;
    }
    return c->second;
}

ItemData ItemManager::get(std::string_view key) {
    return instance().get_item_data(key);
}
ItemData ItemManager::get(ItemID id) { return instance().get_item_data(id); }
ItemID ItemManager::size() { return instance().m_map.size(); }
} // namespace Cubed