#include "Cubed/ui/image.hpp"

#include "Cubed/render/renderer.hpp"
#include "Cubed/texture_manager.hpp"

namespace Cubed {
Image::Image(Widget* parent) : Widget(parent) {}

void Image::on_render(Renderer& renderer) {
    if (m_texture) {
        renderer.render_image(*this);
    }
    Widget::on_render(renderer);
}
void Image::on_update(float dt) { Widget::on_update(dt); }
Image& Image::set_image(const std::string& path,
                        TextureManager& texture_manager, bool change_size) {
    if (m_fill_parent && change_size) {
        Logger::warn("Fill Parent is True don't change size");
    }
    m_texture = texture_manager.get_image_texture(path);
    if (change_size) {
        m_width = m_texture->width();
        m_height = m_texture->height();
    }
    return *this;
}

Image& Image::set_texture(const Texture* texture, bool change_size) {
    if (m_fill_parent && change_size) {
        Logger::warn("Fill Parent is True don't change size");
    }
    m_texture = texture;
    if (change_size) {
        if (m_texture) {
            m_width = m_texture->width();
            m_height = m_texture->height();
        } else {
            m_width = 0;
            m_height = 0;
        }
    }

    return *this;
}

Image& Image::set_scale(float scale) {
    m_scale = scale;
    return *this;
}
float Image::scale() const { return m_scale; }
float Image::height() const {
    if (!m_texture) {
        // Logger::error("Image not set image!");
        return 0.0f;
    }
    if (m_fill_height || m_fill_parent) {
        return m_height;
    }
    return m_height * m_scale;
}

float Image::width() const {
    if (!m_texture) {
        // Logger::error("Image not set image!");
        return 0.0f;
    }
    if (m_fill_parent || m_fill_width) {
        return m_width;
    }
    return m_width * m_scale;
}

const Texture* Image::texture() const { return m_texture; }

} // namespace Cubed