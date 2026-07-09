#pragma once
#include <glad/glad.h>
namespace Cubed {

enum class BufferType : GLenum {
    ARRAY_BUFFER = GL_ARRAY_BUFFER,
    ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER
};

enum class BufferUsage : GLenum {
    STATIC_DRAW = GL_STATIC_DRAW,
    DYNAMIC_DRAW = GL_DYNAMIC_DRAW
};

class VertexBuffer {
public:
    VertexBuffer(BufferType type = BufferType::ARRAY_BUFFER);
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&&) noexcept;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer& operator=(VertexBuffer&&) noexcept;
    ~VertexBuffer();

    void bind() const;
    static void unbind();
    GLuint id() const;
    void buffer_data(const void* data, GLsizeiptr size,
                     BufferUsage usage = BufferUsage::STATIC_DRAW) const;

private:
    GLuint m_vbo = 0;
    BufferType m_type = BufferType::ARRAY_BUFFER;

    GLenum get_buffer_target() const;
};
} // namespace Cubed