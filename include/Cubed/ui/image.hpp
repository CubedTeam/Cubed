#pragma once

#include "Cubed/render/texture.hpp"
#include "Cubed/ui/widget.hpp"
#include "glm/ext/vector_float2.hpp"

#include <filesystem>

namespace Cubed {
class Image : public Widget {
public:
    Image();
    void update(float dt) override;
    void render(Renderer& renderer) override;

    Image& set_image(std::filesystem::path path);
    float width() const;
    float height() const;
    const Texture& texture() const;

private:
    Texture m_texture;
    int m_width = 0;
    int m_height = 0;
    glm::vec2 m_pos{0.0f, 0.0f};
    float m_scale = 1.0f;
};
} // namespace Cubed