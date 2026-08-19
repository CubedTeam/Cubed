#include "Cubed/ui/scroll_view.hpp"

#include "Cubed/render/renderer.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/ui/column_layout.hpp"

#include <algorithm>
#include <glad/glad.h>
namespace cubed {
ScrollView::ScrollView(Widget* parent) : Widget(parent) {}
ScrollView::~ScrollView() {}

void ScrollView::render(Renderer& renderer) {
    if (!m_visible)
        return;
    auto p = pos();
    float h = height();
    float w = width();

    // convert window coords to framebuffer coords for scissor.
    float sx = renderer.window_width() > 0
                   ? renderer.frame_width() / renderer.window_width()
                   : 1.0f;
    float sy = renderer.window_height() > 0
                   ? renderer.frame_height() / renderer.window_height()
                   : 1.0f;

    GLint scissor_x = static_cast<GLint>(p.x * sx);
    GLint scissor_y = static_cast<GLint>((m_window_height - p.y - h) * sy);
    GLsizei scissor_w = static_cast<GLsizei>(std::max(0.0f, w * sx));
    GLsizei scissor_h = static_cast<GLsizei>(std::max(0.0f, h * sy));

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissor_x, scissor_y, scissor_w, scissor_h);

    Widget::render(renderer);

    glDisable(GL_SCISSOR_TEST);
}

float ScrollView::width() const {
    if (Widget::width() > 0) {
        return Widget::width();
    }
    if (m_child) {
        return m_child->width();
    }
    return 0.0f;
}

float ScrollView::height() const {
    if (Widget::height() > 0) {
        return Widget::height();
    }
    Logger::warn("You should set Scroll view height!");
    if (m_parent) {
        return m_parent->height();
    }
    return m_window_height;
}

ScrollView& ScrollView::set_child(std::unique_ptr<ColumnLayout> children) {
    Widget::children().clear();
    m_child = children.get();
    Widget::children().emplace_back(std::move(children));
    m_child->set_offset({0, 0});
    m_child->set_fill_height(false);
    m_child->set_fill_parent(false);
    m_child->set_fill_width(false);
    return *this;
}

bool ScrollView::handle_mouse_move_event(const MouseMoveEvent& e) {
    if (Widget::handle_mouse_move_event(e)) {
        return true;
    }
    auto p = pos();
    if (e.xpos >= p.x && e.xpos <= p.x + width() && e.ypos >= p.y &&
        e.ypos <= p.y + height()) {
        m_hovered = true;
        return true;
    }
    m_hovered = false;
    return false;
}

bool ScrollView::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    if (m_hovered && m_child) {
        float c_height = m_child->height();
        float offset = m_child->get_offset().y;
        float viewport = height();
        float max_offset = std::max(0.0f, c_height - viewport);
        float new_offset =
            std::clamp(offset + e.offset * m_scroll_speed, -max_offset, 0.0f);
        m_child->set_offset({0, new_offset});
        return true;
    }
    return Widget::handle_mouse_wheel_event(e);
}
Widget& ScrollView::set_fill_parent(bool) {
    Logger::warn("You should not use this function in scroll view!");
    return *this;
}
Widget& ScrollView::set_fill_width(bool) {
    Logger::warn("You should not use this function in scroll view!");
    return *this;
}
Widget& ScrollView::set_fill_height(bool) {
    Logger::warn("You should not use this function in scroll view!");
    return *this;
}
} // namespace cubed
