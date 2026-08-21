#include "Cubed/render/model_manager.hpp"

#include "Cubed/gameplay/creatures/creature_manager.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/resource_location.hpp"

#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace fs = std::filesystem;
namespace cubed {

ModelManager& ModelManager::instance() {
    static ModelManager inst;
    return inst;
}

std::optional<ModelManager::Handle>
ModelManager::model(const ResourceLocation& location) {
    return instance().get_model(location);
}

std::optional<ModelManager::Handle> ModelManager::model(ModelID id) {
    return instance().get_model(id);
}

ModelManager::Handle ModelManager::load_model(const ResourceLocation& path,
                                              bool load_anim,
                                              CreatureData* creature_data) {
    auto mhandle = load_model_internal(path);
    if (load_anim && creature_data) {
        load_anim_config(mhandle.node, *creature_data);
    }
    return {mhandle.node, mhandle.id};
}

ModelManager::ModelManager() { init(); }

ModelManager::~ModelManager() {}

void ModelManager::init() {

};

std::optional<ModelManager::Handle>
ModelManager::get_model(const ResourceLocation& location) {
    auto id = get_model_id(location);
    if (id) {
        return get_model(*id);
    } else {
        return std::nullopt;
    }
}

std::optional<ModelManager::Handle> ModelManager::get_model(ModelID id) {

    ModelMap::const_accessor cacc;
    if (m_models.find(cacc, id)) {
        return Handle{cacc->second, cacc->first};
    }

    return std::nullopt;
}

std::optional<ModelID>
ModelManager::get_model_id(const ResourceLocation& name) {
    IDMap::const_accessor cacc;
    if (m_id_map.find(cacc, name)) {
        return cacc->second;
    }
    return std::nullopt;
}

ModelManager::MutableHandle
ModelManager::load_model_internal(const ResourceLocation& location) {
    {
        auto model_id = get_model_id(location);
        if (model_id) {
            ModelMap::accessor acc;
            if (m_models.find(acc, *model_id)) {
                return {acc->second, acc->first};
            }
        }
    }
    fs::path path = location.full_path();

    auto model = m_loader.load(path);

    ModelMap::accessor acc;

    if (m_models.insert(acc, m_next++)) {
        acc->second = std::move(model);
    } else {
        Logger::error("Can't Insert Model {}", location.to_string());
        --m_next;
        auto id = get_model_id(location);
        if (!id) {
            auto err = std::format("Can't get model {} from models map",
                                   location.to_string());
            ASSERT_MSG(false, err);
            throw std::runtime_error(err);
        }
        ModelMap::accessor acc;
        if (m_models.find(acc, *id)) {
            return {acc->second, acc->first};
        }
    }

    m_id_map.emplace(location, acc->first);
    m_location_map.emplace(acc->first, location);

    return {acc->second, acc->first};
}

void ModelManager::load_anim_config(ModelNode& node, const CreatureData& data) {
    if (!data.animation) {
        return;
    }

    fs::path path = data.animation->assets_path_prefix() / data.animation->path;

    rapidjson::Document doc;

    if (!tools::parse_json(doc, path)) {
        return;
    }

    ModelAnimConfig cfg;
    if (doc.HasMember("walk")) {
        cfg.walk_speed = doc["walk"]["speed"].GetFloat();
        cfg.walk_amp = doc["walk"]["amplitude"].GetFloat();
    }
    if (doc.HasMember("run")) {
        cfg.run_speed = doc["run"]["speed"].GetFloat();
        cfg.run_amp = doc["run"]["amplitude"].GetFloat();
    }
    if (doc.HasMember("body_bob")) {
        cfg.body_bob = doc["body_bob"].GetFloat();
    }
    if (doc.HasMember("head")) {
        NodeAnimRule r;
        r.node = doc["head"]["node"].GetString();
        r.role = NodeAnimRule::Role::HEAD;
        cfg.head_amp = doc["head"]["amplitude"].GetFloat();
        cfg.nodes.push_back(std::move(r));
    }
    if (doc.HasMember("legs")) {
        for (auto& leg : doc["legs"].GetArray()) {
            NodeAnimRule r;
            r.node = leg["node"].GetString();
            r.role = NodeAnimRule::Role::LEG;
            r.phase = leg["phase"].GetFloat();
            cfg.nodes.push_back(std::move(r));
        }
    }
    node.anim = std::move(cfg);
}

} // namespace cubed
