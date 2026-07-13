#pragma once
#include "Cubed/ui/widget.hpp"
namespace Cubed {
class ColumnLayout : public Widget {
public:
    ColumnLayout(const ColumnLayout&) = delete;
    ColumnLayout(ColumnLayout&&) = delete;
    ColumnLayout& operator=(const ColumnLayout&) = delete;
    ColumnLayout& operator=(ColumnLayout&&) = delete;
    ColumnLayout(Widget* parent);
    ~ColumnLayout();

    void update(float dt) override;

    void set_spacing(int spacing);
    // No need for parent node pointer; do not modify children's anchors and
    // scale.

    void layout();

private:
    int m_spacing = 0;
};
} // namespace Cubed