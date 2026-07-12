#pragma once

#include "Cubed/render/texture.hpp"
#include "Cubed/ui/widget.hpp"
#include "glm/ext/vector_float2.hpp"

namespace Cubed {
class TextureManager;
class Image : public Widget {
public:
    Image();
    void update(float dt) override;
    void render(Renderer& renderer) override;

    Image& set_image(const std::string& path, TextureManager& texture_manager);
    float width() const;
    float height() const;
    const Texture* texture() const;

private:
    const Texture* m_texture = nullptr;
    glm::vec2 m_pos{0.0f, 0.0f};
    float m_scale = 1.0f;
};
} // namespace Cubed