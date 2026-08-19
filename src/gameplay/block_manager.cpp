#include "Cubed/gameplay/block_manager.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/resource_location.hpp"

#include <algorithm>
#include <filesystem>
#include <rapidjson/document.h>
namespace fs = std::filesystem;
using namespace rapidjson;
using namespace std::string_literals;

namespace cubed {

unsigned BlockManager::sums() {
    ASSERT(is_init);
    return m_datas.size();
}
unsigned BlockManager::cross_plane_sum() {
    ASSERT(is_init);
    return m_cross_plane_map.size();
}

const ResourceLocation& BlockManager::name_form_id(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.name;
    }
    return c->second.name;
}
bool BlockManager::is_gas(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_gas;
    }
    return c->second.is_gas;
}
bool BlockManager::is_liquid(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_liquid;
    }
    return c->second.is_liquid;
}

bool BlockManager::is_water(BlockType id) { return id == m_water; }

bool BlockManager::is_cross_plane(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_cross_plane;
    }
    return c->second.is_cross_plane;
}

bool BlockManager::is_transparent(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_transparent;
    }
    return c->second.is_transparent;
}
bool BlockManager::is_passable(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_passable;
    }
    return c->second.is_passable;
}

bool BlockManager::is_discard(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_discard;
    }
    return c->second.is_discard;
}
bool BlockManager::is_blend(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_blend;
    }
    return c->second.is_blend;
}
bool BlockManager::is_transitional(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.is_transitional;
    }
    return c->second.is_transitional;
}

float BlockManager::roughness(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.roughness;
    }
    return c->second.roughness;
}

void BlockManager::init() {
    fs::path root_path{ResourceLocation::get_assets_path_prefix(
        ResourceLocation::DEFAULT_NAMESPACE)};
    fs::path block_path = root_path / "blocks";

    fs::create_directories(block_path);

    fs::path register_path = root_path / "registry.json";

    Document registry;
    if (!tools::parse_json(registry, register_path)) {
        Logger::error("Can't parse registry.json");
        ASSERT(false);
        return;
    }

    if (!registry.HasMember("blocks")) {
        throw std::runtime_error("registry.json don't has blocks key");
    }

    auto& blocks_registry = registry["blocks"];

    std::vector<std::pair<bool, BlockType>> types;

    for (auto entry : fs::recursive_directory_iterator(
             block_path, fs::directory_options::skip_permission_denied)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().extension() != ".json") {
            continue;
        }

        Document doc;
        if (!tools::parse_json(doc, entry.path())) {
            continue;
        }

        BlockData data;

        std::string path;
        if (!tools::get_json_value(doc, "name", path)) {
            Logger::error("Very Serious Error, Block Name Not Find !!!, Please "
                          "Check The Block Data Integrity");
            continue;
        }
        data.name.path = path;
        data.name.ns = ResourceLocation::DEFAULT_NAMESPACE;
        if (!tools::get_json_value(blocks_registry, path.c_str(), data.id)) {
            Logger::error("Very Serious Error, Block Id Not Find !!!, Please "
                          "Check The Block Data Integrity");
            continue;
        }

        if (!doc.HasMember("properties")) {
            Logger::error("Block {} doesn't have properties",
                          data.name.to_string());
            continue;
        }
        if (!doc["properties"].IsObject()) {
            Logger::error("Block {} properties are not json object",
                          data.name.to_string());
            continue;
        }

        if (data.name.to_string() == "cubed:water") {
            m_water = data.id;
        }

        auto& properties = doc["properties"];

        tools::get_json_value(properties, "is_liquid", data.is_liquid);
        tools::get_json_value(properties, "is_passable", data.is_passable);
        tools::get_json_value(properties, "is_cross_plane",
                              data.is_cross_plane);
        tools::get_json_value(properties, "is_transparent",
                              data.is_transparent);
        tools::get_json_value(properties, "is_gas", data.is_gas);
        tools::get_json_value(properties, "is_discard", data.is_discard);
        tools::get_json_value(properties, "is_blend", data.is_blend);
        tools::get_json_value(properties, "is_transitional",
                              data.is_transitional);
        tools::get_json_value(properties, "roughness", data.roughness);

        if (doc.HasMember("texture") && doc["texture"].IsObject()) {
            auto& texture = doc["texture"];
            std::string s;
            if (tools::get_json_value(texture, "type", s)) {
                if (s == "cuboid") {
                    data.texture_type = BlockTextureType::CUBOID;
                } else if (s == "cross") {
                    data.texture_type = BlockTextureType::CROSS;
                } else {
                    Logger::error("Block {} texture type {} unknown",
                                  data.name.to_string(), s);
                    ASSERT(false);
                }
            } else {
                Logger::error("Block {} need texture type",
                              data.name.to_string());
                ASSERT(false);
                continue;
            }

            if (tools::get_json_value(texture, "path", s)) {
                data.texture_path = ResourceLocation::parse(s);
            }

            if (tools::get_json_value(texture, "normal", s)) {
                data.normal = ResourceLocation::parse(s);
            }
        }

        if (doc.HasMember("sounds") && doc["sounds"].IsObject()) {
            auto& sound = doc["sounds"];
            std::string s;
            if (tools::get_json_value(sound, "break", s)) {
                data.sound.break_s = ResourceLocation::parse(s);
            }
            if (tools::get_json_value(sound, "walk", s)) {
                data.sound.walk = ResourceLocation::parse(s);
            }
            if (tools::get_json_value(sound, "place", s)) {
                data.sound.place = ResourceLocation::parse(s);
            }
        }

        const auto LOCATION = data.name;
        const auto IS_CROSS_PLANE = data.is_cross_plane;
        const auto ID = data.id;
        if (!m_datas.emplace(data.id, std::move(data))) {
            Logger::error("Block {} already exist!", LOCATION.to_string());
            continue;
        }
        m_id_map.emplace(LOCATION, ID);
        types.emplace_back(IS_CROSS_PLANE, ID);
    }
    std::sort(types.begin(), types.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });
    set_up_cross_plane_map(types);
    is_init = true;
}

BlockType BlockManager::cross_plane_index(BlockType id) {
    CrossPlaneMap::const_accessor c;
    if (!m_cross_plane_map.find(c, id)) {
        Logger::error("Can't Find Cross Plane Id {}", id);
        ASSERT(false);
        throw std::out_of_range{"Can't Find Cross Plane Id" +
                                std::to_string(id)};
    }

    return c->second;
}

BlockType BlockManager::id_from_name(std::string_view name) {
    auto s = ResourceLocation::parse(name);
    if (s) {
        return id_from_name(*s);
    }
    return 0;
}

BlockType BlockManager::id_from_name(const ResourceLocation& name) {
    IDMap::const_accessor c;
    if (m_id_map.find(c, name)) {
        return c->second;
    }
    Logger::error("BlockManager: Can't fin Block {}", name.to_string());
    ASSERT(false);
    return 0;
}

BlockData BlockManager::data(std::string_view name) {
    auto s = ResourceLocation::parse(name);
    if (!s) {
        return EMPTY;
    }

    return data(*s);
}

BlockData BlockManager::data(const ResourceLocation& location) {

    IDMap::const_accessor cacc;
    if (!m_id_map.find(cacc, location)) {
        Logger::error("Can't find block {} id", location.to_string());
        ASSERT(false);
        return EMPTY;
    }
    return data(cacc->second);
}

BlockData BlockManager::data(BlockType type) {
    cacc c;
    if (!m_datas.find(c, type)) {
        Logger::error("Can't find block {} data", type);
        ASSERT(false);
        return EMPTY;
    }

    return c->second;
}

void BlockManager::set_up_cross_plane_map(
    const std::vector<std::pair<bool, BlockType>>& types) {
    unsigned cur_id = 0;
    for (auto id : types) {
        if (id.first) {
            m_cross_plane_map.emplace(id.second, cur_id);

            cur_id++;
        }
    }
}

} // namespace cubed
