#include "Cubed/ui/column_layout.hpp"

namespace Cubed {
ColumnLayout::ColumnLayout(Widget* parent) : Widget(parent) {
    Widget::set_order(TraversalOrder::FRONT_TO_BACK);
}
ColumnLayout::~ColumnLayout() {}

void ColumnLayout::on_update(float) { layout(); }

float ColumnLayout::width() const { return m_content_width; }
float ColumnLayout::height() const { return m_content_height; }

ColumnLayout& ColumnLayout::set_spacing(int spacing) {
    m_spacing = spacing;
    return *this;
}
ColumnLayout& ColumnLayout::set_child_anchor(ColumnLayoutAnchor anchor) {
    m_layout_anchor = anchor;
    return *this;
}
void ColumnLayout::layout() {
    auto& children = Widget::children();
    int y = 0;
    Anchor anchor = Anchor::TOP_LEFT;
    switch (m_layout_anchor) {
    case ColumnLayoutAnchor::LEFT:
        anchor = Anchor::TOP_LEFT;
        break;
    case ColumnLayoutAnchor::CENTER:
        anchor = Anchor::TOP_CENTER;
        break;
    case ColumnLayoutAnchor::RIGHT:
        anchor = Anchor::TOP_RIGHT;
        break;
    }
    for (auto& child : children) {
        child->set_anchor(anchor);
        child->set_offset({0, y});
        y += child->height() + m_spacing;
        m_content_width = std::max(m_content_width, child->width());
    }
    m_content_height = children.empty() ? 0 : (y - m_spacing);
}

} // namespace Cubed