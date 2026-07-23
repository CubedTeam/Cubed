#include "Cubed/ui/rect.hpp"

#include "Cubed/render/renderer.hpp"

#include <algorithm>

namespace Cubed {
Rect::Rect(Widget* parent)
    : Widget(parent) {

      };

Rect::~Rect() {}

void Rect::on_update(float dt) { Widget::on_update(dt); }
void Rect::on_render(Renderer& renderer) {
    renderer.render_rect(*this);
    Widget::on_render(renderer);
}

float Rect::width() const {

    if (m_fill_width || m_fill_parent) {
        return Widget::width();
    }
    return Widget::width() * m_scale;
}
float Rect::height() const {

    if (m_fill_height || m_fill_parent) {
        return Widget::height();
    }
    return Widget::height() * m_scale;
}
float Rect::alpha() const { return m_alpha; }

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
bool Rect::supports_border() const { return false; }
} // namespace Cubed