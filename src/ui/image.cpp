#include "Cubed/ui/image.hpp"

#include "Cubed/render/renderer.hpp"
#include "Cubed/texture_manager.hpp"

namespace Cubed {
Image::Image(Widget* parent) : Widget(parent) {}

void Image::on_render(Renderer& renderer) { renderer.render_image(*this); }

Image& Image::set_image(const std::string& path,
                        TextureManager& texture_manager) {
    m_texture = texture_manager.get_image_texture(path);
    return *this;
}
Image& Image::set_scale(float scale) {
    m_scale = scale;
    return *this;
}
float Image::scale() const { return m_scale; }
float Image::height() const {
    if (!m_texture) {
        Logger::error("Image id {} not set image!", m_id);
        return 0.0f;
    }
    return m_texture->height() * m_scale;
}

float Image::width() const {
    if (!m_texture) {
        Logger::error("Image id {} not set image!", m_id);
        return 0.0f;
    }
    return m_texture->width() * m_scale;
}

const Texture* Image::texture() const { return m_texture; }

} // namespace Cubed