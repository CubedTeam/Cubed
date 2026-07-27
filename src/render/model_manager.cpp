#include "Cubed/render/model_manager.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"
namespace Cubed {

ModelManager::ModelManager() {}

ModelManager::~ModelManager() {}

void ModelManager::init() {

};

const ModelNode& ModelManager::get_model(const std::string& model_name) {
    return get_model(get_model_id(model_name));
}

const ModelNode& ModelManager::get_model(ModelID id) {

    ModelMap::const_accessor cacc;
    if (!m_models.find(cacc, id)) {
        return cacc->second;
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
    ASSERT_MSG(false, std::format("ModelManager: Can't find {}", name));
    return 0;
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

const ModelNode& ModelManager::load_model(std::string_view model_name) {
    auto p = model_name.find(':');
    if (p == std::string::npos) {
        Logger::error("Can't Parse Model name {}", model_name);
        ASSERT(false);
    }
    auto name = model_name.substr(p + 1);
    auto space = model_name.substr(0, p);
    if (name.empty()) {
        Logger::error("Can't Parse Model name {}", model_name);
        ASSERT(false);
    }
    std::string path;
    if (space == "cubed") {
        path = std::format("{}model/creature/{}", ASSETS_PATH, name);
    } else {
        path = std::format("./{}/model/creature/{}", space, name);
    }
    auto model = m_loader.load(path);
    ModelMap::accessor acc;
    if (m_models.insert(acc, m_next++)) {
        acc->second = std::move(model);
    } else {
        Logger::error("Can't Insert Model {}", model_name);
        ASSERT(false);
    }
    m_id_map.emplace(model_name, acc->first);
    m_name_map.emplace(acc->first, model_name);
    return acc->second;
}

} // namespace Cubed