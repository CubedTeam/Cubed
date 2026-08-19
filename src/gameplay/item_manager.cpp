#include "Cubed/gameplay/item_manager.hpp"

#include "Cubed/gameplay/block_manager.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"

#include <filesystem>
namespace fs = std::filesystem;
using namespace rapidjson;
namespace cubed {
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
    fs::path registry_path = root_path / "registry.json";
    Document registry;

    if (!tools::parse_json(registry, registry_path)) {
        throw std::runtime_error("registry.json parse error");
    }

    fs::path item_path = root_path / "items";
    fs::create_directories(item_path);
    for (const auto& entry : fs::recursive_directory_iterator(
             item_path, fs::directory_options::skip_permission_denied)) {
        if (fs::is_regular_file(entry)) {
            if (entry.path().extension().string() == ".json") {
                add(entry.path(), registry);
            }
        }
    }
}

void ItemManager::add(const std::filesystem::path& path,
                      const rapidjson::Value& registry) {

    Document doc;
    if (!tools::parse_json(doc, path)) {
        return;
    }

    ItemData data;
    if (!registry.HasMember("items") || !registry["items"].IsObject()) {
        Logger::error("registry.json doesn't have items");
        return;
    }
    auto& items = registry["items"];
    std::string s;
    if (tools::get_json_value(doc, "name", s)) {
        auto location = ResourceLocation::parse(s);
        if (!location) {
            return;
        }
        data.name = *location;
        if (!tools::get_json_value(items, data.name.path.c_str(), data.id)) {
            return;
        }
    } else {
        Logger::error("Item {} doesn't have name", path.string());
        return;
    }

    tools::get_json_value(doc, "description", data.description);

    if (tools::get_json_value(doc, "texture", s)) {
        data.path = ResourceLocation::parse(s);
    }

    if (tools::get_json_value(doc, "type", s)) {
        data.kind = get_item_kind(s);
        if (data.kind == ItemKind::SPAWN_EGG) {
            std::string s;
            if (tools::get_json_value(doc, "creature", s)) {
                auto location = ResourceLocation::parse(s);
                if (location) {
                    data.property = *location;
                } else {
                    data.property = *ResourceLocation::parse("cubed:pig");
                }

            } else {
                data.property = *ResourceLocation::parse("cubed:pig");
            }
        }
    }

    data.local_name =
        tr(std::format("item.{}.{}.name", data.name.ns, data.name.path));

    acc a;
    if (m_map.emplace(a, data.id, std::move(data))) {
        if (!m_id_map.emplace(a->second.name.to_string(), a->first)) {
            Logger::error("ItemManager: Can't Insterd {} {} to id map",
                          a->second.name.to_string(), a->first);
        }
        if (a->second.kind == ItemKind::BLOCK) {
            std::string s;
            if (!tools::get_json_value(doc, "block", s)) {
                return;
            }
            if (s != a->second.name.to_string()) {
                Logger::error("block json name {} != registry.json name {}", s,
                              a->second.name.to_string());
                return;
            }
            BlockType b = BlockManager::id_from_name(a->second.name);
            m_block_to_id_map.emplace(b, a->first);
            a->second.property = b;
        }

    } else {
        Logger::error("ItemManager: Can't Insterd {} to map", path.string());
    }
}

ItemData ItemManager::get_item_data(std::string_view key) const {
    auto location = ResourceLocation::parse(key);
    if (!location) {
        Logger::error("Can't parse key {}", key);
        ASSERT(false);
        return EMPTY;
    }
    ItemID id = 0;
    {
        IDMap::const_accessor cacc;

        if (m_id_map.find(cacc, location->to_string())) {
            id = cacc->second;
        } else {
            Logger::error("Can't Find key {} in id map", location->to_string());
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
} // namespace cubed
