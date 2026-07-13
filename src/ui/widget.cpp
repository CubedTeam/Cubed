#include "Cubed/ui/widget.hpp"

namespace Cubed {
Widget::Widget(const std::string& id) : m_id(id) {}
void Widget::update(float dt) {
    on_update(dt);

    for (auto& child : m_children) {
        child->update(dt);
    }
}

void Widget::render(Renderer& renderer) {
    on_render(renderer);

    for (auto& child : m_children) {
        child->render(renderer);
    }
}

void Widget::on_update(float) {}
void Widget::on_render(Renderer&) {}

Widget& Widget::set_position(const glm::vec2& pos) {
    return set_position(pos.x, pos.y);
}

Widget& Widget::set_position(float x, float y) {
    m_pos.x = x;
    m_pos.y = y;
    return *this;
}

Widget& Widget::set_scale(float scale) {
    m_scale = scale;
    return *this;
}

float Widget::width() const { return 0.0f; }
float Widget::height() const { return 0.0f; }

const glm::vec2& Widget::pos() const { return m_pos; }
float Widget::scale() const { return m_scale; }
const std::string& Widget::id() const { return m_id; }

bool Widget::handle_key_event(const KeyEvent& e) {
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handle_key_event(e)) {
            return true;
        }
    }

    return false;
}
bool Widget::handle_mouse_button_event(const MouseButtonEvent& e) {
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handle_mouse_button_event(e)) {
            return true;
        }
    }
    return false;
}
bool Widget::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handle_mouse_wheel_event(e)) {
            return true;
        }
    }
    return false;
}
bool Widget::handle_window_resize_event(const WindowResizeEvent& e) {
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handle_window_resize_event(e)) {
            return true;
        }
    }
    return false;
}
bool Widget::handle_mouse_move_event(const MouseMoveEvent& e) {
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->handle_mouse_move_event(e)) {
            return true;
        }
    }
    return false;
}
} // namespace Cubed