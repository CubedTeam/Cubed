#include "Cubed/ui/image.hpp"

#include "Cubed/render/renderer.hpp"
#include "Cubed/texture_manager.hpp"

namespace Cubed {
Image::Image(Widget* parent) : Widget(parent) {}

void Image::on_render(Renderer& renderer) { renderer.render_image(*this); }
void Image::on_update(float) {
    if (m_fill) {
        if (m_fill) {
            if (!m_parent) {
                m_width = m_window_width;
                m_height = m_window_height;
            } else {
                m_width = m_parent->width();
                m_height = m_parent->height();
            }
        }
    }
}
Image& Image::set_image(const std::string& path,
                        TextureManager& texture_manager) {
    m_texture = texture_manager.get_image_texture(path);
    m_width = m_texture->width();
    m_height = m_texture->height();
    return *this;
}
Image& Image::set_scale(float scale) {
    m_scale = scale;
    return *this;
}
float Image::scale() const { return m_scale; }
float Image::height() const {
    if (!m_texture) {
        Logger::error("Image not set image!");
        return 0.0f;
    }
    return m_height * m_scale;
}

float Image::width() const {
    if (!m_texture) {
        Logger::error("Image not set image!");
        return 0.0f;
    }
    return m_width * m_scale;
}

Image& Image::set_height(float height) {
    m_height = height;
    return *this;
}
Image& Image ::set_width(float width) {
    m_width = width;
    return *this;
}

Image& Image::set_fill(bool fill) {
    m_fill = fill;
    return *this;
}

const Texture* Image::texture() const { return m_texture; }

} // namespace Cubed