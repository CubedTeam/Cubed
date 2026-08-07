#pragma once

#include "Cubed/primitive_data.hpp"
#include "Cubed/render/texture.hpp"
#include "Cubed/render/vertex_array.hpp"
#include "Cubed/render/vertex_buffer.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <string_view>
namespace Cubed {

struct Mesh {
    std::vector<Vertex3D> vertices;
    std::vector<uint32_t> indices;
    std::unique_ptr<VertexBuffer> vbo;
    std::unique_ptr<VertexBuffer> ebo;
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<Texture> texture;
    void upload() {
        vao = std::make_unique<VertexArray>();
        vao->bind();
        vbo = std::make_unique<VertexBuffer>();
        vbo->buffer_data(vertices.data(), vertices.size() * sizeof(Vertex3D));
        ebo = std::make_unique<VertexBuffer>(BufferType::ELEMENT_ARRAY_BUFFER);
        ebo->buffer_data(indices.data(), indices.size() * sizeof(uint32_t));
        vao->attribute(0, 3, GL_FLOAT, sizeof(Vertex3D), (void*)0);
        vao->attribute(1, 2, GL_FLOAT, sizeof(Vertex3D),
                       (void*)offsetof(Vertex3D, s));
        vao->attribute(2, 3, GL_FLOAT, sizeof(Vertex3D),
                       (void*)offsetof(Vertex3D, nx));
    };
};
struct NodeAnimRule {
    enum class Role { NONE, LEG, HEAD };
    std::string node;
    Role role = Role::NONE;
    float phase = 0.0f;
};

struct ModelAnimConfig {
    float walk_speed = 6.0f;
    float walk_amp = 25.0f;
    float run_speed = 12.0f;
    float run_amp = 40.0f;
    float body_bob = 0.05f;
    float head_amp = 4.0f;
    std::vector<NodeAnimRule> nodes;

    const NodeAnimRule* rule_for(std::string_view name) const {
        for (const auto& r : nodes) {
            if (r.node == name) {
                return &r;
            }
        }
        return nullptr;
    }

    bool has_role(NodeAnimRule::Role role) const {
        for (const auto& r : nodes) {
            if (r.role == role) {
                return true;
            }
        }
        return false;
    }
};
struct ModelNode {
    std::string name;
    glm::mat4 transform{1.0f};

    std::vector<Mesh> meshes;
    std::vector<ModelNode> children;
    ModelAnimConfig anim;
};

} // namespace Cubed