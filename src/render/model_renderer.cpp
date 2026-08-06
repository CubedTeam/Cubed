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

void ModelRender::render_instance(ModelID id,
                                  std::span<const InstanceData> instances,
                                  const Camera& camera, bool shadow) {
    if (instances.empty()) {
        return;
    }
    auto& root = ModelManager::model(id).node;
    auto& batch = get_batch(id, root);
    const size_t PER_INSTANCE_SIZE = batch.node_count * sizeof(glm::mat4);
    if (instances.size() > batch.capacity) {
        batch.capacity = instances.size() * 2;
        batch.instance_matrices.resize(batch.capacity * batch.node_count);
        batch.instance_vbo = std::make_unique<VertexBuffer>();
        batch.instance_vbo->buffer_data(batch.instance_matrices.data(),
                                        batch.instance_matrices.size() *
                                            sizeof(glm::mat4),
                                        BufferUsage::DYNAMIC_DRAW);
        for (const auto& entry : batch.entries) {
            for (int col = 0; col < 4; ++col) {
                entry.mesh->vao->attribute(
                    3 + col, 4, GL_FLOAT, PER_INSTANCE_SIZE,
                    (void*)(entry.node_slot * sizeof(glm::mat4) +
                            col * sizeof(glm::vec4)));
                entry.mesh->vao->divisor(3 + col);
            }
        }
    }

    for (size_t i = 0; i < instances.size(); ++i) {
        glm::vec3 bob_pos = instances[i].pos;
        if (instances[i].pose.gait != Gait::STOP &&
            root.anim.has_role(NodeAnimRule::Role::LEG)) {
            float speed = instances[i].pose.gait == Gait::RUN
                              ? root.anim.run_speed
                              : root.anim.walk_speed;
            bob_pos.y +=
                std::abs(glm::sin(instances[i].pose.walk_time * speed)) *
                root.anim.body_bob;
        }

        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), bob_pos) *
            glm::rotate(glm::mat4(1.0f), instances[i].yaw, {0, 1, 0});
        collect_matrices(root, transform, instances[i].pose, root.anim,
                         batch.instance_matrices, i * batch.node_count);
    }

    batch.instance_vbo->buffer_sub_data(
        batch.instance_matrices.data(),
        instances.size() * batch.node_count * sizeof(glm::mat4), 0);

    auto& shader = shadow ? m_renderer.get_shader("depth_model_instance")
                          : m_renderer.get_shader("model_instance");
    glm::mat4 view = camera.get_camera_lookat();
    shader.set_loc("proj_matrix", m_renderer.p_mat());
    if (shadow) {
    } else {
        shader.set_loc("view_matrix", view);
    }
    for (const auto& entry : batch.entries) {
        if (entry.mesh->texture) {
            entry.mesh->texture->bind(1);
        }
        entry.mesh->vao->bind();
        glDrawElementsInstanced(GL_TRIANGLES, entry.mesh->indices.size(),
                                GL_UNSIGNED_INT, 0, instances.size());
    }
}

size_t ModelRender::collect_matrices(const ModelNode& node,
                                     const glm::mat4& parent,
                                     const WalkPose& pose,
                                     const ModelAnimConfig& cfg,
                                     std::vector<glm::mat4>& out, size_t slot) {
    glm::mat4 transform = parent * pose_node(node, cfg, pose);
    out[slot] = transform;
    size_t next = slot + 1;
    for (const auto& clild : node.children) {
        next = collect_matrices(clild, transform, pose, cfg, out, next);
    }
    return next;
}

ModelRender::ModelBatch& ModelRender::get_batch(ModelID id,
                                                const ModelNode& root) {
    auto it = m_batches.find(id);
    if (it != m_batches.end()) {
        return it->second;
    }
    ModelBatch batch;
    batch.node_count = flatten_nodes(root, 0, batch);
    return m_batches.try_emplace(id, std::move(batch)).first->second;
}

size_t ModelRender::flatten_nodes(const ModelNode& node, size_t slot,
                                  ModelBatch& batch) {
    size_t node_slot = slot++;
    for (const auto& mesh : node.meshes) {
        batch.entries.emplace_back(&mesh, node_slot);
    }

    for (const auto& child : node.children) {
        slot = flatten_nodes(child, slot, batch);
    }

    return slot;
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