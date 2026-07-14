#include "Cubed/ui/rect.hpp"

#include "Cubed/render/renderer.hpp"
namespace Cubed {
Rect::Rect(Widget* parent)
    : Widget(parent) {

      };

Rect::~Rect() {}

void Rect::on_update(float) {
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
void Rect::on_render(Renderer& renderer) { renderer.render_rect(*this); }

float Rect::width() const { return m_width * m_scale; }
float Rect::height() const { return m_height * m_scale; }
float Rect::alpha() const { return m_alpha; }
Rect& Rect::set_width(float width) {
    m_width = width;
    return *this;
}
Rect& Rect::set_height(float height) {
    m_height = height;
    return *this;
}

Rect& Rect::set_fill(bool fill) {
    m_fill = fill;
    return *this;
}

Rect& Rect::set_scale(float scale) {
    m_scale = scale;
    return *this;
}
Rect& Rect::set_color(Color color) {
    m_color = color;
    return *this;
}
Rect& Rect::set_alpha(float alpha) {
    m_alpha = std::clamp(alpha, 0.0f, 1.0f);
    return *this;
}
Color Rect::color() const { return m_color; }
} // namespace Cubed