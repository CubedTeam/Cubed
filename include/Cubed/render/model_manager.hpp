#pragma once
#include "Cubed/gameplay/model.hpp"
#include "Cubed/render/model_node.hpp"
#include "Cubed/tools/model_loader.hpp"
#include "Cubed/tools/resource_location.hpp"

#include <tbb/concurrent_hash_map.h>
namespace cubed {
class CreatureData;
class ModelManager {
public:
    struct Handle {
        const ModelNode& node;
        ModelID id = 0;
    };
    struct MutableHandle {
        ModelNode& node;
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
    std::optional<Handle> get_model(const ResourceLocation& location);
    [[nodiscard]]
    std::optional<Handle> get_model(ModelID id);
    [[nodiscard]]
    static std::optional<Handle> model(const ResourceLocation& location);
    [[nodiscard]]
    static std::optional<Handle> model(ModelID id);

    Handle load_model(const ResourceLocation& path, bool load_anim,
                      CreatureData* creature_data = nullptr);
    std::optional<ModelID> get_model_id(const ResourceLocation& location);
    void init();

private:
    ModelLoader m_loader;
    ModelID m_next = 0;
    using ModelMap = tbb::concurrent_hash_map<ModelID, ModelNode>;
    using IDMap = tbb::concurrent_hash_map<ResourceLocation, ModelID,
                                           ResourceLocation::Hash>;
    using LocationMap = tbb::concurrent_hash_map<ModelID, ResourceLocation>;
    ModelMap m_models;
    IDMap m_id_map;
    LocationMap m_location_map;

    void load_anim_config(ModelNode& node, const CreatureData& data);
    MutableHandle load_model_internal(const ResourceLocation& location);
};
} // namespace cubed
