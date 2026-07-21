#pragma once

#include "Cubed/render/texture.hpp"
#include "Cubed/ui/widget.hpp"
#include "glm/ext/vector_float2.hpp"

namespace Cubed {
class TextureManager;
class Image : public Widget {
public:
    Image(const Image&) = delete;
    Image(Image&&) = delete;
    Image& operator=(const Image&) = delete;
    Image& operator=(Image&&) = delete;
    Image(Widget* parent);

    Image& set_image(const std::string& path, TextureManager& texture_manager);
    float width() const override;
    float height() const override;
    const Texture* texture() const;
    Image& set_scale(float scale);
    float scale() const;

private:
    void on_render(Renderer& renderer) override;
    void on_update(float dt) override;

    const Texture* m_texture = nullptr;
    glm::vec2 m_pos{0.0f, 0.0f};
    float m_scale = 1.0f;
};
} // namespace Cubed