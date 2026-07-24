#include "Cubed/render/model_manager.hpp"

#include "Cubed/tools/log.hpp"
namespace Cubed {

ModelManager::ModelManager() {}

ModelManager::~ModelManager() {}

void ModelManager::init() {

};

const ModelNode& ModelManager::get_model(const std::string& model_name) {
    auto it = m_models.find(model_name);
    if (it == m_models.end()) {
        return load_model(model_name);
    }
    return it->second;
}

const ModelNode& ModelManager::load_model(const std::string& model_name) {
    std::string path = ASSETS_PATH + model_name;
    auto model = m_loader.load(path);
    auto [it, instert] = m_models.try_emplace(model_name, std::move(model));
    if (!instert) {
        Logger::error("Model Key {} already exist!", model_name);
    }
    return it->second;
}

} // namespace Cubed