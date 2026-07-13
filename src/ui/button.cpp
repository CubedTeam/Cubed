#include "Cubed/ui/button.hpp"

namespace Cubed {
Button::Button() {}

void Button::update(float dt) {
    if (m_background) {
        m_background->update(dt);
    }
    if (m_foreground) {
        m_foreground->update(dt);
    }
}

void Button::render(Renderer& renderer) {
    if (m_background) {
        m_background->render(renderer);
    }
    if (m_foreground) {
        m_foreground->render(renderer);
    }
}

bool Button::handle_mouse_move_event(const MouseMoveEvent& e) {

    if (e.xpos >= m_min_pos.x && e.xpos <= m_max_pos.x) {
        if (e.ypos >= m_min_pos.y && e.ypos <= m_max_pos.y) {
            m_hovered = true;

            return true;
        }
    }
    m_hovered = false;

    return false;
}
bool Button::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {
        if (m_hovered && m_clicked) {
            m_clicked();

            return true;
        }
    }
    return false;
}

Widget& Button::set_position(const glm::vec2& pos) {
    m_pos = pos;
    m_background->set_position(pos);
    m_foreground->set_position(pos.x, pos.y + m_foreground->height() / 2.0f);
    m_min_pos = pos;

    m_max_pos.x = m_min_pos.x + width();
    m_max_pos.y = m_min_pos.y + height();

    return *this;
}
Widget& Button::set_position(float x, float y) {
    return set_position(glm::vec2{x, y});
}
Widget& Button::set_scale(float scale) {
    m_scale = scale;
    m_background->set_scale(scale);
    m_max_pos.x = m_min_pos.x + width();
    m_max_pos.y = m_min_pos.y + height();
    return *this;
}
float Button::width() const { return m_background->width() * m_scale; }
float Button::height() const { return m_background->height() * m_scale; }

} // namespace Cubed