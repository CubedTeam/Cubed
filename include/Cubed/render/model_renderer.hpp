#pragma once

#include "Cubed/render/model_node.hpp"
#include "Cubed/shader.hpp"

#include <glm/glm.hpp>
namespace Cubed {
class Renderer;
class Camera;
class ModelRender {
public:
    ModelRender(Renderer& renderer);
    void render_model(const std::string& name, const glm::vec3& pos,
                      Camera& camera);

private:
    Renderer& m_renderer;
    void render_node(const ModelNode& node, const glm::mat4& parent,
                     const glm::mat4& view, const Shader& shader);
    void render_mesh(const Mesh& mesh);
};
} // namespace Cubed