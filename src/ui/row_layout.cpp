#include "Cubed/ui/row_layout.hpp"

namespace Cubed {
RowLayout::RowLayout(Widget* parent) : Widget(parent) {}
RowLayout::~RowLayout() {}
void RowLayout::on_update(float) { layout(); }

float RowLayout::width() const { return m_content_width; }
float RowLayout::height() const { return m_content_height; }

RowLayout& RowLayout::set_spacing(int spacing) {
    m_spacing = spacing;
    return *this;
}
void RowLayout::layout() {
    auto& children = Widget::children();
    int x = 0;
    for (auto& child : children) {
        child->set_anchor(Anchor::TOP_LEFT);
        child->set_offset({x, 0});
        x += child->width() + m_spacing;
        m_content_height = std::max(m_content_height, child->height());
    }
    m_content_width = children.empty() ? 0 : (x - m_spacing);
}
} // namespace Cubed