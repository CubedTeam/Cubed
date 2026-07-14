#include "Cubed/ui/button.hpp"

namespace Cubed {
Button::Button(Widget* parent) : Widget(parent) {}

void Button::on_update(float dt) {
    if (m_background) {
        m_background->update(dt);
    }
    if (m_foreground) {
        m_foreground->update(dt);
    }
}

void Button::on_render(Renderer& renderer) {
    if (m_background) {
        m_background->render(renderer);
    }
    if (m_foreground) {
        m_foreground->render(renderer);
    }
}

bool Button::handle_mouse_move_event(const MouseMoveEvent& e) {
    auto pos = m_background->pos();
    if (e.xpos >= pos.x && e.xpos <= pos.x + width() && e.ypos >= pos.y &&
        e.ypos <= pos.y + height()) {
        m_hovered = true;
        return true;
    }
    m_hovered = false;

    return Widget::handle_mouse_move_event(e);
}
bool Button::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {
        if (m_hovered && m_clicked) {
            m_clicked();

            return true;
        }
    }

    return Widget::handle_mouse_button_event(e);
}

Button& Button::set_scale(float scale) {
    m_background->set_scale(scale);
    return *this;
}
void Button::set_window_size(int width, int height) {
    m_background->set_window_size(width, height);
    Widget::set_window_size(width, height);
}
Widget& Button::set_anchor(Anchor anchor) {
    m_background->set_anchor(anchor);

    return *this;
}
Widget& Button::set_offset(glm::ivec2 offset) {
    m_background->set_offset(offset);

    return *this;
}
glm::vec2 Button::pos() const { return m_background->pos(); }

float Button::scale() const { return m_background->scale(); }

float Button::width() const { return m_background->width(); }
float Button::height() const { return m_background->height(); }

} // namespace Cubed