#include "Cubed/render/model_manager.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/resource_location.hpp"

#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

namespace fs = std::filesystem;
namespace Cubed {

ModelManager& ModelManager::instance() {
    static ModelManager inst;
    return inst;
}

[[nodiscard]]
ModelManager::Handle ModelManager::model(const std::string& model_name) {
    return instance().get_model(model_name);
}
[[nodiscard]]
ModelManager::Handle ModelManager::model(ModelID id) {
    return instance().get_model(id);
}

ModelManager::ModelManager() { init(); }

ModelManager::~ModelManager() {}

void ModelManager::init() {

};

ModelManager::Handle ModelManager::get_model(const std::string& model_name) {
    return get_model(get_model_id(model_name));
}

ModelManager::Handle ModelManager::get_model(ModelID id) {

    ModelMap::const_accessor cacc;
    if (m_models.find(cacc, id)) {
        return {cacc->second, cacc->first};
    }
    return load_model(get_model_name(id));
}

const ModelNode& get_model(const std::string& model_name);
const ModelNode& get_model(ModelID id);
ModelID ModelManager::get_model_id(const std::string& name) {
    IDMap::const_accessor cacc;
    if (m_id_map.find(cacc, name)) {
        return cacc->second;
    }
    return load_model(name).id;
}

const std::string& ModelManager::get_model_name(ModelID id) {
    NameMap::const_accessor cacc;
    if (m_name_map.find(cacc, id)) {
        return cacc->second;
    }
    ASSERT_MSG(false, std::format("ModelManager: Can't find {}", id));
    static std::string n = "";
    return n;
}

ModelManager::Handle ModelManager::load_model(std::string_view model_name) {

    auto space = ResourceLocation::parse(model_name);
    if (!space) {
        Logger::error("Can't Parse Model name {}", model_name);
        ASSERT(false);
    }
    std::string path;
    if (space->ns == "cubed") {
        path = std::format("{}model/creature/{}/{}.glb", ASSETS_PATH,
                           space->path, space->path);
    } else {
        path = std::format("./{}/model/creature/{}/{}.glb", space->ns,
                           space->path, space->path);
    }
    auto model = m_loader.load(path);
    fs::path anim_path = path;
    anim_path = anim_path.parent_path() / "animation.json";
    load_anim_config(model, anim_path.string());
    ModelMap::accessor acc;
    if (m_models.insert(acc, m_next++)) {
        acc->second = std::move(model);
    } else {
        Logger::error("Can't Insert Model {}", model_name);
        ModelMap::const_accessor cacc;
        if (m_models.find(cacc, get_model_id(std::string(model_name)))) {
            return {cacc->second, cacc->first};
        }
    }
    m_id_map.emplace(model_name, acc->first);
    m_name_map.emplace(acc->first, model_name);
    return {acc->second, acc->first};
}

void ModelManager::load_anim_config(ModelNode& node, const std::string& path) {
    if (!fs::is_regular_file(path)) {
        return;
    }
    std::ifstream s(path);
    if (!s.is_open()) {
        return;
    }
    rapidjson::IStreamWrapper isw(s);
    rapidjson::Document doc;
    doc.ParseStream(isw);
    if (doc.HasParseError()) {
        Logger::warn("Can't parse anim config {}", path);
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

} // namespace Cubed