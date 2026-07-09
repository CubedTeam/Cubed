#include "Cubed/render/texture.hpp"

#include "Cubed/tools/cubed_assert.hpp"

#include <utility>

namespace Cubed {
Texture::Texture(TextureType type) : M_TYPE(type) { glGenTextures(1, &m_id); }
Texture::~Texture() {
    if (m_id) {
        glDeleteTextures(1, &m_id);
    }
}
Texture::Texture(Texture&& o) noexcept
    : m_id(std::exchange(o.m_id, 0)), M_TYPE(o.M_TYPE) {}

Texture& Texture::operator=(Texture&& o) noexcept {
    if (this == &o) {
        return *this;
    }
    if (M_TYPE != o.M_TYPE) {
        ASSERT_MSG(false, "Texture Type is not same");
    }
    if (m_id) {
        glDeleteTextures(1, &m_id);
    }

    m_id = std::exchange(o.m_id, 0);
    return *this;
}
void Texture::bind() const { glBindTexture(get_gl_texture_type(), m_id); }

void Texture::bind(size_t unit) const {
    active(unit);
    bind();
}

GLuint Texture::id() const { return m_id; }

void Texture::parameter(TexturePname pname, TextureParam param) const {
    bind();
    glTexParameteri(get_gl_texture_type(), std::to_underlying(pname),
                    std::to_underlying(param));
}
void Texture::parameterfv(TexturePname pname, const float* param) const {
    bind();
    glTexParameterfv(get_gl_texture_type(), std::to_underlying(pname), param);
}
void Texture::tex_image_2d(TextureFormat internalformat, TextureFormat format,
                           GLenum type, const void* data, GLsizei width,
                           GLsizei height, GLint level, GLint border) const {
    bind();
    glTexImage2D(get_gl_texture_type(), level,
                 std::to_underlying(internalformat), width, height, border,
                 std::to_underlying(format), type, data);
}

void Texture::tex_image_3d(TextureFormat internalformat, TextureFormat format,
                           GLenum type, const void* data, GLsizei width,
                           GLsizei height, GLsizei depth, GLint level,
                           GLint border) const {
    bind();
    glTexImage3D(get_gl_texture_type(), level,
                 std::to_underlying(internalformat), width, height, depth,
                 border, std::to_underlying(format), type, data);
}
void Texture::tex_sub_image_3d(TextureFormat format, GLenum type,
                               const void* data, GLint xoffset, GLint yoffset,
                               GLint zoffset, GLsizei width, GLsizei height,
                               GLsizei depth, GLint level) const {
    bind();
    glTexSubImage3D(get_gl_texture_type(), level, xoffset, yoffset, zoffset,
                    width, height, depth, std::to_underlying(format), type,
                    data);
}
void Texture::set_aniso(int aniso) const {
    if (aniso >= 1) {
        bind();
        glTexParameterf(get_gl_texture_type(), GL_TEXTURE_MAX_ANISOTROPY,
                        static_cast<GLfloat>(aniso));
    }
}

void Texture::gen_mipmap() const {
    bind();
    glGenerateMipmap(get_gl_texture_type());
}

void Texture::set_linear() const {
    parameter(TexturePname::MIN_FILTER, TextureParam::LINEAR);
    parameter(TexturePname::MAG_FILTER, TextureParam::LINEAR);
}
void Texture::set_nearest_and_minpmap() const {
    parameter(TexturePname::MAG_FILTER, TextureParam::NEAREST);
    parameter(TexturePname::MIN_FILTER, TextureParam::LINEAR_MIPMAP_LINEAR);
    gen_mipmap();
}
void Texture::set_nearest() const {
    parameter(TexturePname::MAG_FILTER, TextureParam::NEAREST);
    parameter(TexturePname::MIN_FILTER, TextureParam::NEAREST);
}
void Texture::set_repeat(bool r, bool s, bool t) const {
    if (r) {
        parameter(TexturePname::WRAP_R, TextureParam::REPEAT);
    }
    if (s) {
        parameter(TexturePname::WRAP_S, TextureParam::REPEAT);
    }
    if (t) {
        parameter(TexturePname::WRAP_T, TextureParam::REPEAT);
    }
}
void Texture::set_clamp_to_border(bool r, bool s, bool t) const {
    if (r) {
        parameter(TexturePname::WRAP_R, TextureParam::CLAMP_TO_BORDER);
    }
    if (s) {
        parameter(TexturePname::WRAP_S, TextureParam::CLAMP_TO_BORDER);
    }
    if (t) {
        parameter(TexturePname::WRAP_T, TextureParam::CLAMP_TO_BORDER);
    }
}

void Texture::set_clamp_to_edge(bool r, bool s, bool t) const {
    if (r) {
        parameter(TexturePname::WRAP_R, TextureParam::CLAMP_TO_EDGE);
    }
    if (s) {
        parameter(TexturePname::WRAP_S, TextureParam::CLAMP_TO_EDGE);
    }
    if (t) {
        parameter(TexturePname::WRAP_T, TextureParam::CLAMP_TO_EDGE);
    }
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
void Texture::active(size_t id) { glActiveTexture(GL_TEXTURE0 + id); }

GLenum Texture::get_gl_texture_type() const {
    return std::to_underlying(M_TYPE);
}

} // namespace Cubed