#include "Cubed/ui/column_layout.hpp"

namespace Cubed {
ColumnLayout::ColumnLayout(Widget* parent) : Widget(parent) {}
ColumnLayout::~ColumnLayout() {}

void ColumnLayout::update(float dt) {
    Widget::update(dt);
    layout();
}

float ColumnLayout::width() const {
    if (m_parent) {
        return m_parent->width();
    }
    return m_window_width;
}
float ColumnLayout::height() const {
    if (m_parent) {
        return m_parent->height();
    }
    return m_window_height;
}

void ColumnLayout::set_spacing(int spacing) { m_spacing = spacing; }

void ColumnLayout::layout() {
    auto& children = Widget::children();
    int y = 0;
    for (auto& child : children) {
        child->set_offset({0, y});
        child->set_anchor(m_anchor);
        y += child->height() + m_spacing;
    }
}

} // namespace Cubed