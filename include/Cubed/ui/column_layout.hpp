#pragma once
#include "Cubed/ui/widget.hpp"
namespace Cubed {

enum class ColumnLayoutAnchor { LEFT, CENTER, RIGHT };

class ColumnLayout : public Widget {
public:
    ColumnLayout(const ColumnLayout&) = delete;
    ColumnLayout(ColumnLayout&&) = delete;
    ColumnLayout& operator=(const ColumnLayout&) = delete;
    ColumnLayout& operator=(ColumnLayout&&) = delete;
    ColumnLayout(Widget* parent);
    ~ColumnLayout();
    float width() const override;
    float height() const override;

    ColumnLayout& set_spacing(int spacing);
    // No need for parent node pointer; do not modify children's anchors and
    // scale.
    ColumnLayout& set_child_anchor(ColumnLayoutAnchor anchor);
    void layout();

private:
    int m_spacing = 0;
    float m_content_height = 0.0f;
    float m_content_width = 0.0f;
    ColumnLayoutAnchor m_layout_anchor = ColumnLayoutAnchor::CENTER;

    void on_update(float dt) override;
};
} // namespace Cubed