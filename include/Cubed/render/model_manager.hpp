#pragma once
#include "Cubed/render/model_node.hpp"
#include "Cubed/tools/model_loader.hpp"
namespace Cubed {
class ModelManager {
public:
    ModelManager();
    ModelManager(const ModelManager&) = delete;
    ModelManager(ModelManager&&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;
    ModelManager& operator=(ModelManager&&) = delete;
    ~ModelManager();
    const ModelNode& get_model(const std::string& model_name);
    void init();

private:
    ModelLoader m_loader;
    std::unordered_map<std::string, ModelNode> m_models;
    const ModelNode& load_model(const std::string& model_name);
};
} // namespace Cubed