#pragma once
#include <cstddef>
#include <glad/glad.h>
namespace Cubed {

enum TextureType : GLenum {
    TEXTURE_2D = GL_TEXTURE_2D,
    TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY
};

enum TexturePname : GLenum {
    MIN_FILTER = GL_TEXTURE_MIN_FILTER,
    MAG_FILTER = GL_TEXTURE_MAG_FILTER,
    WRAP_S = GL_TEXTURE_WRAP_S,
    WRAP_T = GL_TEXTURE_WRAP_T,
    WRAP_R = GL_TEXTURE_WRAP_R,
    BORDER_COLOR = GL_TEXTURE_BORDER_COLOR,
    COMPARE_MODE = GL_TEXTURE_COMPARE_MODE
};

enum TextureParam : GLenum {
    LINEAR = GL_LINEAR,
    NEAREST = GL_NEAREST,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
    CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    T_NONE = GL_NONE,
    REPEAT = GL_REPEAT
};

enum TextureFormat : GLenum {
    DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    R16F = GL_R16F,
    RGBA16F = GL_RGBA16F,
    RED = GL_RED,
    RGBA = GL_RGBA,
    RGB = GL_RGB,
    RGBA8 = GL_RGBA8,

};

class Texture {
public:
    explicit Texture(TextureType type);
    ~Texture();
    Texture(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) noexcept;

    void bind() const;
    void bind(size_t unit) const;
    static void unbind();
    static void active(size_t id);
    GLuint id() const;

    void parameter(TexturePname pname, TextureParam param) const;
    void parameterfv(TexturePname pname, const float* param) const;

    void tex_image_2d(TextureFormat internalformat, TextureFormat format,
                      GLenum type, const void* data, GLsizei width,
                      GLsizei height, GLint level = 0, GLint border = 0) const;

    void tex_image_3d(TextureFormat internalformat, TextureFormat format,
                      GLenum type, const void* data, GLsizei width,
                      GLsizei height, GLsizei depth, GLint level = 0,
                      GLint border = 0) const;

    void tex_sub_image_3d(TextureFormat format, GLenum type, const void* data,
                          GLint xoffset, GLint yoffset, GLint zoffset,
                          GLsizei width, GLsizei height, GLsizei depth = 1,
                          GLint level = 0) const;

    void set_aniso(int aniso) const;

    void gen_mipmap() const;

    void set_linear() const;
    void set_nearest_and_minpmap() const;
    void set_nearest() const;
    void set_repeat(bool r = true, bool s = true, bool t = true) const;
    void set_clamp_to_border(bool r = true, bool s = true, bool t = true) const;
    void set_clamp_to_edge(bool r = true, bool s = true, bool t = true) const;

    TextureType type() const;

private:
    GLuint m_id = 0;
    const TextureType M_TYPE;

    GLenum get_gl_texture_type() const;
};
} // namespace Cubed