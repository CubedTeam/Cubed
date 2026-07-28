#include "Cubed/render/model_renderer.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/render/model_manager.hpp"
#include "Cubed/render/renderer.hpp"
namespace Cubed {
ModelRender::ModelRender(Renderer& renderer) : m_renderer(renderer) {}

void ModelRender::render_model(ModelID id, const glm::vec3& pos,
                               Camera& camera) {
    auto& root = ModelManager::model(id).node;
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
    auto& shader = m_renderer.get_shader("model_shader");
    glm::mat4 view = camera.get_camera_lookat();
    shader.set_loc("proj_matrix", m_renderer.p_mat());
    render_node(root, transform, view, shader, false);
}

void ModelRender::shadow_pass(ModelID id, const glm::vec3& pos,
                              Camera& camera) {
    auto& root = ModelManager::model(id).node;
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
    auto& shader = m_renderer.get_shader("depth_model");
    glm::mat4 view = camera.get_camera_lookat();

    render_node(root, transform, view, shader, true);
}

void ModelRender::render_node(const ModelNode& node, const glm::mat4& parent,
                              const glm::mat4& view, const Shader& shader,
                              bool shadow) {

    glm::mat4 transform = parent * node.transform;
    if (shadow) {
        shader.set_loc("modelMatrix", transform);
    } else {
        glm::mat4 mv_matrix = view * transform;
        shader.set_loc("modelMatrix", transform);
        shader.set_loc("mv_matrix", mv_matrix);
        shader.set_loc("norm_matrix", glm::transpose(glm::inverse(mv_matrix)));
    }

    for (auto& mesh : node.meshes) {
        render_mesh(mesh, shadow);
    }

    for (auto& child : node.children) {
        render_node(child, transform, view, shader, shadow);
    }
}

void ModelRender::render_mesh(const Mesh& mesh, bool) {
    mesh.vao->bind();
    if (mesh.texture) {
        mesh.texture->bind(1);
    } else {
        Logger::error("Model Texture Not Find");
    }

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()),
                   GL_UNSIGNED_INT, 0);
}

} // namespace Cubed