#include "Cubed/render/model_renderer.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/render/model_manager.hpp"
#include "Cubed/render/renderer.hpp"
namespace Cubed {
ModelRender::ModelRender(Renderer& renderer) : m_renderer(renderer) {}

void ModelRender::render_model(const std::string& name, const glm::vec3& pos,
                               Camera& camera) {
    auto& model_manager = m_renderer.model_manager();
    auto& root = model_manager.get_model(name);
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
    auto& shader = m_renderer.get_shader("model_shader");
    glm::mat4 view = camera.get_camera_lookat();
    shader.use();
    shader.set_loc("proj_matrix", m_renderer.p_mat());
    render_node(root, transform, view, shader);
}

void ModelRender::render_node(const ModelNode& node, const glm::mat4& parent,
                              const glm::mat4& view, const Shader& shader) {

    glm::mat4 transform = parent * node.transform;

    shader.set_loc("mv_matrix", view * transform);
    for (auto& mesh : node.meshes) {
        render_mesh(mesh);
    }

    for (auto& child : node.children) {
        render_node(child, transform, view, shader);
    }
}

void ModelRender::render_mesh(const Mesh& mesh) {
    mesh.vao->bind();
    if (mesh.texture) {
        mesh.texture->bind(0);
    } else {
        Logger::error("Model Texture Not Find");
    }

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()),
                   GL_UNSIGNED_INT, 0);
}

} // namespace Cubed