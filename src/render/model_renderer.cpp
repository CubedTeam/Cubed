#include "Cubed/render/model_renderer.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/render/model_manager.hpp"
#include "Cubed/render/renderer.hpp"

#include <cmath>
namespace Cubed {

namespace {
float swing_angle(const WalkPose& pose, float speed, float amp_deg) {
    if (pose.gait == Gait::STOP) {
        return 0.0f;
    }
    return glm::sin(pose.walk_time * speed) * glm::radians(amp_deg);
}
} // namespace

ModelRender::ModelRender(Renderer& renderer) : m_renderer(renderer) {}

void ModelRender::render_model(ModelID id, const glm::vec3& pos, float yaw,
                               Camera& camera, const WalkPose& pose) {
    auto& root = ModelManager::model(id).node;
    glm::vec3 bob_pos = pos;
    if (pose.gait != Gait::STOP &&
        root.anim.has_role(NodeAnimRule::Role::LEG)) {
        bob_pos.y += std::abs(glm::sin(pose.walk_time * root.anim.walk_speed)) *
                     root.anim.body_bob;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), bob_pos) *
                          glm::rotate(glm::mat4(1.0f), yaw, {0, 1, 0});
    auto& shader = m_renderer.get_shader("model_shader");
    glm::mat4 view = camera.get_camera_lookat();
    shader.set_loc("proj_matrix", m_renderer.p_mat());
    render_node(root, transform, view, shader, false, pose, root.anim);
}

void ModelRender::shadow_pass(ModelID id, const glm::vec3& pos, float yaw,
                              Camera& camera, const WalkPose& pose) {
    auto& root = ModelManager::model(id).node;
    glm::vec3 bob_pos = pos;
    if (pose.gait != Gait::STOP &&
        root.anim.has_role(NodeAnimRule::Role::LEG)) {
        bob_pos.y += std::abs(glm::sin(pose.walk_time * root.anim.walk_speed)) *
                     root.anim.body_bob;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), bob_pos) *
                          glm::rotate(glm::mat4(1.0f), yaw, {0, 1, 0});

    auto& shader = m_renderer.get_shader("depth_model");
    glm::mat4 view = camera.get_camera_lookat();

    render_node(root, transform, view, shader, true, pose, root.anim);
}

void ModelRender::render_node(const ModelNode& node, const glm::mat4& parent,
                              const glm::mat4& view, const Shader& shader,
                              bool shadow, const WalkPose& pose,
                              const ModelAnimConfig& cfg) {

    glm::mat4 transform = parent * pose_node(node, cfg, pose);
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
        render_node(child, transform, view, shader, shadow, pose, cfg);
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

glm::mat4 ModelRender::pose_node(const ModelNode& node,
                                 const ModelAnimConfig& cfg,
                                 const WalkPose& pose) {
    const auto* rule = cfg.rule_for(node.name);
    if (!rule) {
        return node.transform;
    }
    float angle = 0.0f;
    if (rule->role == NodeAnimRule::Role::LEG) {
        float speed = pose.gait == Gait::RUN ? cfg.run_speed : cfg.walk_speed;
        float amp = pose.gait == Gait::RUN ? cfg.run_amp : cfg.walk_amp;

        angle = swing_angle(pose, speed, amp);
        angle *= std::cos(rule->phase);
    } else if (rule->role == NodeAnimRule::Role::HEAD) {
        angle = glm::sin(pose.walk_time * cfg.walk_speed * 0.5f) *
                glm::radians(cfg.head_amp);
    } else {
        return node.transform;
    }
    return glm::rotate(node.transform, angle, glm::vec3(1.0f, 0.0f, 0.0f));
}

} // namespace Cubed