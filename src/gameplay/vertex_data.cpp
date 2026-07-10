#include "Cubed/gameplay/vertex_data.hpp"

#include "Cubed/gameplay/client_world.hpp"

namespace Cubed {
VertexData::VertexData(ClientWorld& world) : m_world(world) {}
VertexData::~VertexData() {

    m_world.push_delete_vbo(m_vbo);

    m_world.push_delete_vao(m_vao);
}
VertexData::VertexData(VertexData&& o) noexcept
    : m_vertices(std::move(o.m_vertices)), m_vbo(std::move(o.m_vbo)),
      m_vao(std::move(o.m_vao)), m_sum(o.m_sum.exchange(0)),
      m_world(o.m_world) {}
VertexData& VertexData::operator=(VertexData&& o) noexcept {
    if (this == &o) {
        return *this;
    }

    m_world.push_delete_vao(m_vao);
    m_world.push_delete_vbo(m_vbo);

    m_vbo = std::move(o.m_vbo);
    m_vao = std::move(o.m_vao);

    m_sum = o.m_sum.exchange(0);

    m_vertices = std::move(o.m_vertices);

    return *this;
}
void VertexData::upload() {
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
    m_vbo->buffer_data(m_vertices.data(), m_vertices.size() * sizeof(Vertex3D),
                       BufferUsage::DYNAMIC_DRAW);

    m_vao->attribute(0, 3, GL_FLOAT, sizeof(Vertex3D), (void*)0);
    m_vao->attribute(1, 2, GL_FLOAT, sizeof(Vertex3D),
                     (void*)offsetof(Vertex3D, s));
    m_vao->attribute(2, 1, GL_FLOAT, sizeof(Vertex3D),
                     (void*)offsetof(Vertex3D, layer));
    m_vao->attribute(3, 3, GL_FLOAT, sizeof(Vertex3D),
                     (void*)offsetof(Vertex3D, nx));
    m_vao->attribute(4, 1, GL_FLOAT, sizeof(Vertex3D),
                     (void*)offsetof(Vertex3D, roughness));
    m_vao->attribute(5, 3, GL_FLOAT, sizeof(Vertex3D),
                     (void*)offsetof(Vertex3D, tx));

    VertexArray::unbind();
    VertexBuffer::unbind();

    // Release memory
    m_vertices.clear();
}
void VertexData::update_sum() { m_sum = m_vertices.size(); }
} // namespace Cubed