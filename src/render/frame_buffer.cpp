#include "Cubed/render/frame_buffer.hpp"

#include "Cubed/tools/log.hpp"

#include <utility>

namespace Cubed {
FrameBuffer::FrameBuffer() { glGenFramebuffers(1, &m_fbo); }
FrameBuffer::~FrameBuffer() {
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
    }
}

FrameBuffer::FrameBuffer(FrameBuffer&& o) noexcept
    : m_fbo(std::exchange(o.m_fbo, 0)) {}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& o) noexcept {
    if (this == &o) {
        return *this;
    }
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
    }
    m_fbo = std::exchange(o.m_fbo, 0);
    return *this;
}

GLuint FrameBuffer::id() const { return m_fbo; }

void FrameBuffer::bind(FrameBufferType type) const {
    glBindFramebuffer(std::to_underlying(type), m_fbo);
}
void FrameBuffer::unbind() {

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void FrameBuffer::attach(Attachment attachment, const Texture& texture,
                         GLint level) const {
    bind();
    auto type = texture.type();
    if (type == TextureType::TEXTURE_2D) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, std::to_underlying(attachment),
                               std::to_underlying(type), texture.id(), level);
    }
}

bool FrameBuffer::check_status() const {
    bind();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Logger::error("FBO incomplete after resize!");
        return false;
    } else {
        Logger::info("Frame Buffer Complete!");
        return true;
    }
}

void FrameBuffer::draw_buffer(GLenum buf) const {
    bind();
    glDrawBuffer(buf);
}
void FrameBuffer::read_buffer(GLenum src) const {
    bind();
    glReadBuffer(src);
}
void FrameBuffer::draw_buffer(GLsizei n, const GLenum* bufs) const {
    bind();
    glDrawBuffers(n, bufs);
}

void FrameBuffer::draw_buffer(std::span<const GLenum> bufs) const {
    bind();
    glDrawBuffers(bufs.size(), bufs.data());
}

} // namespace Cubed