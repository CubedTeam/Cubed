#include "Cubed/gameplay/block_manager.hpp"

#include "Cubed/localization.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/toml.utils.hpp"

#include <filesystem>

namespace fs = std::filesystem;

using namespace std::string_literals;
using namespace Cubed::TOML;
namespace {
std::string block_data_dir = ASSETS_PATH + "data/block"s;

} // namespace

namespace Cubed {

unsigned BlockManager::sums() {
    ASSERT(is_init);
    return m_datas.size();
}
unsigned BlockManager::cross_plane_sum() {
    ASSERT(is_init);
    return m_cross_plane_map.size();
}

const std::string& BlockManager::name_form_id(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.name;
    }
    return c->second.name;
}
std::string BlockManager::local_name(BlockType id) {
    cacc c;
    if (!m_datas.find(c, id)) {
        ASSERT(false);
        return EMPTY.name_key;
    }
    return tr(c->second.name_key);
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
    fs::path data_path{block_data_dir};

    for (auto entry : fs::recursive_directory_iterator(data_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().filename() == "template.toml") {
            continue;
        }
        toml::table block;
        try {
            block = toml::parse_file(entry.path().string());
        } catch (const toml::parse_error& err) {
            Logger::error("Load Block Data {} Fail, Parser Error {}",
                          entry.path().string(), err.what());
            ASSERT(false);
        }
        auto id = block["id"].value<int>();
        if (id == std::nullopt) {
            Logger::error("Very Serious Error, Block Id Not Find !!!, Please "
                          "Check The Block Data Integrity");
            std::abort();
        }
        auto name = block["name"].value<std::string>();
        if (name == std::nullopt) {
            Logger::error("Very Serious Error, Block Name Not Find !!!, Please "
                          "Check The Block Data Integrity");
            std::abort();
        }
        auto is_liquid = safe_get_value(block, "is_liquid", false);
        auto is_passable = safe_get_value(block, "is_passable", false);
        auto is_cross_plane = safe_get_value(block, "is_cross_plane", false);
        auto is_transparent = safe_get_value(block, "is_transparent", false);
        auto is_gas = safe_get_value(block, "is_gas", false);
        auto is_discard = safe_get_value(block, "is_discard", false);
        auto is_blend = safe_get_value(block, "is_blend", false);
        auto is_transitional = safe_get_value(block, "is_transitional", false);
        auto roughness = safe_get_value(block, "roughness", 1.0);
        auto name_key = safe_get_value(block, "name_key", "Unknow_key");
        BlockData data{
            *name,           *name_key,        static_cast<BlockType>(*id),
            *is_liquid,      *is_passable,     *is_cross_plane,
            *is_transparent, *is_gas,          *is_discard,
            *is_blend,       *is_transitional, static_cast<float>(*roughness)};

        if (!m_datas.emplace(static_cast<BlockType>(*id), std::move(data))) {
            Logger::error("Block Type {} already exist!", *id);
        }
    }

    set_up_cross_plane_map();
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

BlockType BlockManager::id_from_name(const std::string& name) {
    IDMap::const_accessor c;
    if (m_id_map.find(c, name)) {
        return c->second;
    }
    Logger::error("BlockManager: Can't fin Block {}", name);
    ASSERT(false);
    return 0;
}

void BlockManager::set_up_cross_plane_map() {
    unsigned cur_id = 0;
    for (const auto& [id, data] : m_datas) {
        if (data.is_cross_plane) {
            m_cross_plane_map.emplace(data.id, cur_id);

            cur_id++;
        }
    }
}

} // namespace Cubed