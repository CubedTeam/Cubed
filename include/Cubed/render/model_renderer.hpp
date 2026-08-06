#pragma once

#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/model.hpp"
#include "Cubed/render/model_node.hpp"
#include "Cubed/shader.hpp"

#include <glm/glm.hpp>
namespace Cubed {
class Renderer;
class Camera;
class ModelRender {
public:
    ModelRender(Renderer& renderer);
    void render_model(ModelID id, const glm::vec3& pos, float yaw,
                      Camera& camera, const WalkPose& pose);

    void shadow_pass(ModelID id, const glm::vec3& pos, float yaw,
                     Camera& camera, const WalkPose& pose);

private:
    Renderer& m_renderer;
    void render_node(const ModelNode& node, const glm::mat4& parent,
                     const glm::mat4& view, const Shader& shader, bool shadow,
                     const WalkPose& pose, const ModelAnimConfig& cfg);
    glm::mat4 pose_node(const ModelNode& node, const ModelAnimConfig& cfg,
                        const WalkPose& pose);
    void render_mesh(const Mesh& mesh, bool shadow);
};
} // namespace Cubed