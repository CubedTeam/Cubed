#pragma once

#include "Cubed/primitive_data.hpp"
#include "Cubed/render/texture.hpp"
#include "Cubed/render/vertex_array.hpp"
#include "Cubed/render/vertex_buffer.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
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
struct ModelNode {
    std::string name;
    glm::mat4 transform{1.0f};
    std::vector<Mesh> meshes;
    std::vector<ModelNode> children;
};

} // namespace Cubed