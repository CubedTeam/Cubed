#pragma once
#include <glad/glad.h>

namespace cubed {
class VertexArray {
public:
    VertexArray();
    VertexArray(const VertexArray&) = delete;
    VertexArray(VertexArray&&) noexcept;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray& operator=(VertexArray&&) noexcept;
    ~VertexArray();

    void bind() const;

    static void unbind();

    GLuint id() const;

    void attribute(GLuint index, GLint size, GLenum type, GLsizei stride,
                   const void* ptr, bool normalized = false) const;
    void divisor(GLuint index, GLuint divisor = 1);

private:
    GLuint m_vao = 0;
};
} // namespace cubed
