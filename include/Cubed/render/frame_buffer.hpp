#pragma once
#include "Cubed/render/texture.hpp"

#include <glad/glad.h>
#include <span>

enum class Attachment : GLenum {
    COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
    COLOR_ATTACHMENT1 = GL_COLOR_ATTACHMENT1,
    DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT
};

enum class FrameBufferType : GLenum {
    FRAMEBUFFER = GL_FRAMEBUFFER,
    READ_FRAMEBUFFER = GL_READ_FRAMEBUFFER,
    DRAW_FRAMEBUFFER = GL_DRAW_FRAMEBUFFER
};

namespace Cubed {
class FrameBuffer {
public:
    FrameBuffer();
    ~FrameBuffer();
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&&) noexcept;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    FrameBuffer& operator=(FrameBuffer&&) noexcept;

    void bind(FrameBufferType type = FrameBufferType::FRAMEBUFFER) const;
    GLuint id() const;

    void attach(Attachment attachment, const Texture& texture,
                GLint level = 0) const;
    static void unbind();

    bool check_status() const;

    void draw_buffer(GLenum buf) const;
    void read_buffer(GLenum src) const;
    void draw_buffer(GLsizei n, const GLenum* bufs) const;
    void draw_buffer(std::span<const GLenum> bufs) const;

private:
    GLuint m_fbo = 0;
};
} // namespace Cubed