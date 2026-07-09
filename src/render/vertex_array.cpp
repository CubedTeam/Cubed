#include "Cubed/render/vertex_array.hpp"

#include <utility>

namespace Cubed {
VertexArray::VertexArray() { glGenVertexArrays(1, &m_vao); }
VertexArray::~VertexArray() {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }
}
VertexArray::VertexArray(VertexArray&& o) noexcept
    : m_vao(std::exchange(o.m_vao, 0)) {}

VertexArray& VertexArray::operator=(VertexArray&& o) noexcept {
    if (this != &o) {
        if (m_vao) {
            glDeleteVertexArrays(1, &m_vao);
        }
        m_vao = std::exchange(o.m_vao, 0);
    }
    return *this;
}

void VertexArray::bind() const { glBindVertexArray(m_vao); }
void VertexArray::unbind() { glBindVertexArray(0); }

GLuint VertexArray::id() const { return m_vao; }

void VertexArray::attribute(GLuint index, GLint size, GLenum type,
                            GLsizei stride, const void* ptr,
                            bool normalized) const {
    bind();
    glVertexAttribPointer(index, size, type, normalized ? GL_TRUE : GL_FALSE,
                          stride, ptr);
    glEnableVertexAttribArray(index);
}

} // namespace Cubed