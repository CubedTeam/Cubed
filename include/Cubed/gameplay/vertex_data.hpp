#pragma once
#include "Cubed/primitive_data.hpp"
#include "Cubed/render/vertex_array.hpp"
#include "Cubed/render/vertex_buffer.hpp"

#include <atomic>
#include <glad/glad.h>
#include <memory>
#include <vector>
namespace Cubed {
class ClientWorld;
struct VertexData {
    std::vector<Vertex3D> m_vertices;
    std::unique_ptr<VertexBuffer> m_vbo;
    std::unique_ptr<VertexArray> m_vao;
    std::atomic<std::size_t> m_sum{0};
    ClientWorld& m_world;
    VertexData(ClientWorld& world);
    ~VertexData();
    VertexData(const VertexData&) = delete;
    VertexData(VertexData&&) noexcept;
    VertexData& operator=(const VertexData&) = delete;
    VertexData& operator=(VertexData&&) noexcept;

    void upload();
    void update_sum();
};
} // namespace Cubed
