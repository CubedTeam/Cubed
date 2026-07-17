#include "Cubed/ui/widget.hpp"

#include "Cubed/tools/log.hpp"

namespace Cubed {
Widget::Widget(Widget* parent) : m_parent(parent) {}
void Widget::update(float dt) {
    on_update(dt);

    for (auto& child : m_children) {
        child->update(dt);
    }
}

void Widget::render(Renderer& renderer) {
    if (!m_visible) {
        return;
    }
    on_render(renderer);

    for (auto& child : m_children) {
        child->render(renderer);
    }
}

void Widget::on_update(float) {}
void Widget::on_render(Renderer&) {}

std::vector<std::unique_ptr<Widget>>& Widget::children() { return m_children; }

const std::vector<std::unique_ptr<Widget>>& Widget::children() const {
    return m_children;
}

glm::vec2 Widget::compute_position() const {
    glm::vec2 pos{0, 0};
    float parent_w;
    float parent_h;
    if (!m_parent) {
        if (m_window_height == 0 || m_window_width == 0) {
            Logger::error("Window Size is 0 !");
        }
        parent_h = m_window_height;
        parent_w = m_window_width;
    } else {
        parent_h = m_parent->height();
        parent_w = m_parent->width();
    }
    const float W = width();
    const float H = height();
    switch (m_anchor) {
    case Anchor::TOP_LEFT:
        pos = {0, 0};
        break;

    case Anchor::TOP_CENTER:
        pos = {(parent_w - W) / 2, 0};
        break;

    case Anchor::TOP_RIGHT:
        pos = {parent_w - W, 0};
        break;

    case Anchor::CENTER_LEFT:
        pos = {0, (parent_h - H) / 2};
        break;

    case Anchor::CENTER:
        pos = {(parent_w - W) / 2, (parent_h - H) / 2};
        break;

    case Anchor::CENTER_RIGHT:
        pos = {parent_w - W, (parent_h - H) / 2};
        break;

    case Anchor::BOTTOM_LEFT:
        pos = {0, parent_h - H};
        break;

    case Anchor::BOTTOM_CENTER:
        pos = {(parent_w - W) / 2, parent_h - H};
        break;

    case Anchor::BOTTOM_RIGHT:
        pos = {parent_w - W, parent_h - H};
        break;
    }
    if (m_parent) {
        pos += m_parent->pos();
    }
    pos += m_offset;
    return pos;
}

Widget& Widget::set_anchor(Anchor anchor) {
    m_anchor = anchor;
    return *this;
}
Widget& Widget::set_offset(glm::ivec2 offset) {
    m_offset = offset;
    return *this;
}
Widget& Widget::set_window_size(int width, int height) {
    m_window_height = height;
    m_window_width = width;
    return *this;
}

Widget& Widget::set_visible(bool visible) {
    m_visible = visible;
    return *this;
}

float Widget::width() const {
    if (m_parent) {
        return m_parent->width();
    }
    return m_window_width;
}
float Widget::height() const {
    if (m_parent) {
        return m_parent->height();
    }
    return m_window_height;
}

glm::vec2 Widget::pos() const { return compute_position(); }

bool Widget::handle_key_event(const KeyEvent& e) {
    if (m_order == TraversalOrder::BACK_TO_FRONT) {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handle_key_event(e)) {
                return true;
            }
        }
    } else if (m_order == TraversalOrder::FRONT_TO_BACK) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if ((*it)->handle_key_event(e)) {
                return true;
            }
        }
    }

    return false;
}
bool Widget::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (m_order == TraversalOrder::BACK_TO_FRONT) {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handle_mouse_button_event(e)) {
                return true;
            }
        }
    } else if (m_order == TraversalOrder::FRONT_TO_BACK) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if ((*it)->handle_mouse_button_event(e)) {
                return true;
            }
        }
    }

    return false;
}
bool Widget::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    if (m_order == TraversalOrder::BACK_TO_FRONT) {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handle_mouse_wheel_event(e)) {
                return true;
            }
        }
    } else if (m_order == TraversalOrder::FRONT_TO_BACK) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if ((*it)->handle_mouse_wheel_event(e)) {
                return true;
            }
        }
    }
    return false;
}
bool Widget::handle_window_resize_event(const WindowResizeEvent& e) {

    set_window_size(e.width, e.height);

    if (m_order == TraversalOrder::BACK_TO_FRONT) {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handle_window_resize_event(e)) {
                return true;
            }
        }
    } else if (m_order == TraversalOrder::FRONT_TO_BACK) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if ((*it)->handle_window_resize_event(e)) {
                return true;
            }
        }
    }
    return false;
}

bool Widget::handle_mouse_move_event(const MouseMoveEvent& e) {
    if (m_order == TraversalOrder::BACK_TO_FRONT) {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handle_mouse_move_event(e)) {
                return true;
            }
        }
    } else if (m_order == TraversalOrder::FRONT_TO_BACK) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if ((*it)->handle_mouse_move_event(e)) {
                return true;
            }
        }
    }
    return false;
}

bool Widget::handle_text_input_event(const TextInputEvent& e) {
    if (m_order == TraversalOrder::BACK_TO_FRONT) {
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handle_text_input_event(e)) {
                return true;
            }
        }
    } else if (m_order == TraversalOrder::FRONT_TO_BACK) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if ((*it)->handle_text_input_event(e)) {
                return true;
            }
        }
    }
    return false;
}
void Widget::set_order(TraversalOrder order) { m_order = order; }
} // namespace Cubed