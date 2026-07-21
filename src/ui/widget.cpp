#include "Cubed/ui/widget.hpp"

#include "Cubed/tools/log.hpp"
#include "Cubed/ui/rect.hpp"

namespace Cubed {
Widget::Widget(Widget* parent) : m_parent(parent) {
    if (!m_parent) {
        // The root node of the window automatically fills the entire window.
        set_fill_parent(true);
    }
}
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
    if (m_show_border) {
        for (auto& w : m_border) {
            if (w) {
                w->render(renderer);
            }
        }
    }
}

void Widget::on_update(float) {

    if (m_fill_width) {
        set_width(m_parent ? m_parent->width() : m_window_width);
    }
    if (m_fill_height) {
        set_height(m_parent ? m_parent->height() : m_window_height);
    }
    if (m_fill_parent) {
        set_width(m_parent ? m_parent->width() : m_window_width);
        set_height(m_parent ? m_parent->height() : m_window_height);
    }
}
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

void Widget::set_width_internal(float width) { m_width = width; }
void Widget::set_height_internal(float height) { m_height = height; }

void Widget::update_border() {
    if (!supports_border()) {
        return;
    }
    auto w = width();
    auto h = height();
    for (int i = 0; i < 4; ++i) {
        if (!m_border[i]) {
            auto rect = std::make_unique<Rect>(this);
            rect->set_anchor(Anchor::TOP_LEFT);
            rect->set_color(Color::WHITE);
            m_border[i] = std::move(rect);
        }
    }

    m_border[0]->set_width(w).set_height(m_border_size);
    m_border[1]
        ->set_width(w)
        .set_height(m_border_size)
        .set_offset({0, h - m_border_size});
    m_border[2]->set_height(h).set_width(m_border_size);
    m_border[3]
        ->set_height(h)
        .set_width(m_border_size)
        .set_offset({w - m_border_size, 0});
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

float Widget::width() const { return m_width; }
float Widget::height() const { return m_height; }

Widget& Widget::set_width(float width) {
    set_width_internal(width);
    if (supports_border()) {
        update_border();
    }

    return *this;
}
Widget& Widget::set_height(float height) {
    set_height_internal(height);
    if (supports_border()) {
        update_border();
    }

    return *this;
}

Widget& Widget::set_fill_parent(bool fill) {
    m_fill_parent = fill;
    return *this;
}
Widget& Widget::set_fill_width(bool fill) {
    m_fill_width = fill;
    return *this;
}
Widget& Widget::set_fill_height(bool fill) {
    m_fill_height = fill;
    return *this;
}

Widget& Widget::set_border_size(int size) {
    if (size < 0) {
        return *this;
    }
    if (m_border_size == size) {
        return *this;
    }
    m_border_size = size;
    update_border();
    return *this;
}
Widget& Widget::set_border_visale(bool visable) {
    m_show_border = visable;

    return *this;
}
bool Widget::supports_border() const { return true; }
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