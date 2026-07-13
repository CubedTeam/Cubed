#include "Cubed/ui/button.hpp"

#include "Cubed/tools/log.hpp"

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
            Logger::info("Hovered {}", m_hovered);
            return true;
        }
    }
    m_hovered = false;
    Logger::info("Hovered {}", m_hovered);
    return false;
}
bool Button::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {
        if (m_hovered && m_clicked) {
            m_clicked();
            Logger::info("Button click!");
            return true;
        }
    }
    return false;
}

Widget& Button::set_position(const glm::vec2& pos) {
    m_pos = pos;
    m_background->set_position(pos);
    m_foreground->set_position(pos);
    m_min_pos = pos;

    m_max_pos.x = m_min_pos.x + m_background->width();
    m_max_pos.y = m_min_pos.y + m_background->height();

    return *this;
}
Widget& Button::set_position(float x, float y) {
    return set_position(glm::vec2{x, y});
}

float Button::width() const { return m_background->width(); }
float Button::height() const { return m_background->height(); }

} // namespace Cubed