#pragma once
#include "Cubed/primitive_data.hpp"
#include "Cubed/render/vertex_array.hpp"
#include "Cubed/render/vertex_buffer.hpp"

#include <atomic>
#include <memory>
#include <vector>
namespace Cubed {
struct UIVertexData {
    std::vector<Vertex2D> m_vertices;
    std::unique_ptr<VertexBuffer> m_vbo;
    std::unique_ptr<VertexArray> m_vao;
    std::atomic<std::size_t> m_sum{0};
    UIVertexData();
    ~UIVertexData();
    UIVertexData(const UIVertexData&) = delete;
    UIVertexData(UIVertexData&&) noexcept;
    UIVertexData& operator=(const UIVertexData&) = delete;
    UIVertexData& operator=(UIVertexData&&) noexcept;

    void upload();
    void update_sum();
};
} // namespace Cubed
