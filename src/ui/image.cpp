#include "Cubed/ui/image.hpp"

#include "Cubed/render/renderer.hpp"
#include "Cubed/tools/shader_tools.hpp"

namespace Cubed {
Image::Image() : m_texture(TextureType::TEXTURE_2D) {}
void Image::update(float dt) { (void)dt; }
void Image::render(Renderer& renderer) { renderer.render_image(*this); }

Image& Image::set_image(std::filesystem::path path) {
    auto data = Tools::load_image_data(path);
    m_texture.tex_image_2d(RGBA, RGBA, GL_UNSIGNED_BYTE, data.data, data.width,
                           data.height);
    m_height = data.height;
    m_width = data.width;
    m_texture.set_nearest();
    return *this;
}

float Image::height() const { return m_height; }

float Image::width() const { return m_width; }

const Texture& Image::texture() const { return m_texture; }

} // namespace Cubed