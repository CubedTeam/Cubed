#pragma once
#include "Cubed/gameplay/model.hpp"
#include "Cubed/render/model_node.hpp"
#include "Cubed/tools/model_loader.hpp"

#include <tbb/concurrent_hash_map.h>
namespace cubed {
class CreatureData;
class ModelManager {
public:
    struct Handle {
        const ModelNode& node;
        ModelID id = 0;
    };

    ModelManager();
    ModelManager(const ModelManager&) = delete;
    ModelManager(ModelManager&&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;
    ModelManager& operator=(ModelManager&&) = delete;
    static ModelManager& instance();
    ~ModelManager();
    [[nodiscard]]
    Handle get_model(const std::string& model_name);
    [[nodiscard]]
    Handle get_model(ModelID id);
    [[nodiscard]]
    static Handle model(const std::string& model_name);
    [[nodiscard]]
    static Handle model(ModelID id);
    ModelID get_model_id(const std::string& name);
    const std::string& get_model_name(ModelID id);
    void init();

private:
    ModelLoader m_loader;
    ModelID m_next = 0;
    using ModelMap = tbb::concurrent_hash_map<ModelID, ModelNode>;
    using IDMap = tbb::concurrent_hash_map<std::string, ModelID>;
    using NameMap = tbb::concurrent_hash_map<ModelID, std::string>;
    ModelMap m_models;
    IDMap m_id_map;
    NameMap m_name_map;
    Handle load_model(std::string_view model_name);
    void load_anim_config(ModelNode& node, const CreatureData& data);
};
} // namespace cubed
