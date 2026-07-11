
#include "Cubed/ui/ui_vertex_data.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"
namespace Cubed {
UIVertexData::UIVertexData() {}
UIVertexData::~UIVertexData() {
    m_vao.reset();
    m_vbo.reset();
}

void UIVertexData::upload() {
    if (m_sum == 0) {
        Logger::error("You need update_sum first");
        ASSERT(false);
    }
    if (m_vertices.size() == 0) {
        return;
    }
    if (!m_vao) {
        m_vao = std::make_unique<VertexArray>();
    }
    if (!m_vbo) {
        m_vbo = std::make_unique<VertexBuffer>();
    }
    m_vao->bind();
    m_vbo->buffer_data(m_vertices.data(), m_vertices.size() * sizeof(Vertex2D),
                       BufferUsage::DYNAMIC_DRAW);

    m_vao->attribute(0, 3, GL_FLOAT, sizeof(Vertex2D), (void*)0);
    m_vao->attribute(1, 2, GL_FLOAT, sizeof(Vertex2D),
                     (void*)offsetof(Vertex2D, s));
    m_vao->attribute(2, 1, GL_FLOAT, sizeof(Vertex2D),
                     (void*)offsetof(Vertex2D, layer));

    VertexArray::unbind();
    VertexBuffer::unbind();

    // Release memory
    m_vertices.clear();
}

void UIVertexData::update_sum() { m_sum = m_vertices.size(); }

} // namespace Cubed