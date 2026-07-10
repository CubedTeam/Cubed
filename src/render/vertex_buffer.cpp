#include "Cubed/render/vertex_buffer.hpp"

#include <utility>

namespace Cubed {
VertexBuffer::VertexBuffer(BufferType type) : m_type(type) {
    glGenBuffers(1, &m_vbo);
}
VertexBuffer::~VertexBuffer() {
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
    }
}

VertexBuffer::VertexBuffer(VertexBuffer&& o) noexcept
    : m_vbo(std::exchange(o.m_vbo, 0)), m_type(o.m_type) {}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& o) noexcept {
    if (this != &o) {
        if (m_vbo) {
            glDeleteBuffers(1, &m_vbo);
        }
        m_vbo = std::exchange(o.m_vbo, 0);
        m_type = o.m_type;
    }

    return *this;
}

void VertexBuffer::bind() const { glBindBuffer(get_buffer_target(), m_vbo); }

void VertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLuint VertexBuffer::id() const { return m_vbo; }

GLenum VertexBuffer::get_buffer_target() const {
    return std::to_underlying(m_type);
}

void VertexBuffer::buffer_data(const void* data, GLsizeiptr size,
                               BufferUsage usage) const {
    bind();

    GLenum target = get_buffer_target();

    glBufferData(target, size, data, std::to_underlying(usage));
}

} // namespace Cubed