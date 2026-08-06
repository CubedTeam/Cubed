#pragma once

#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/model.hpp"
#include "Cubed/render/model_node.hpp"

#include <glm/glm.hpp>
#include <span>
namespace Cubed {
class Renderer;
class Camera;
class ModelRender {
public:
    struct InstanceData {
        glm::vec3 pos{0.0f};
        float yaw = 0.0f;
        WalkPose pose;
    };

    struct DrawEntry {
        const Mesh* mesh = nullptr;
        size_t node_slot = 0;
    };

    struct ModelBatch {
        size_t node_count = 0;
        size_t capacity = 0;
        std::vector<DrawEntry> entries;
        std::vector<glm::mat4> instance_matrices;
        std::unique_ptr<VertexBuffer> instance_vbo;
    };

    ModelRender(Renderer& renderer);

    void render_instance(ModelID id, std::span<const InstanceData> instances,
                         const Camera& camera, bool shadow);

private:
    Renderer& m_renderer;

    std::unordered_map<ModelID, ModelBatch> m_batches;

    size_t collect_matrices(const ModelNode& node, const glm::mat4& parent,
                            const WalkPose& pose, const ModelAnimConfig& cfg,
                            std::vector<glm::mat4>& out, size_t slot);
    glm::mat4 pose_node(const ModelNode& node, const ModelAnimConfig& cfg,
                        const WalkPose& pose);

    ModelBatch& get_batch(ModelID id, const ModelNode& root);
    size_t flatten_nodes(const ModelNode& node, size_t slot, ModelBatch& batch);
};
} // namespace Cubed