#include "Cubed/ui/column_layout.hpp"

namespace Cubed {
ColumnLayout::ColumnLayout(Widget* parent) : Widget(parent) {}
ColumnLayout::~ColumnLayout() {}

void ColumnLayout::update(float dt) {
    Widget::update(dt);
    layout();
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