#pragma once

#include "Cubed/ui/widget.hpp"
namespace Cubed {
class RowLayout : public Widget {
public:
    RowLayout(Widget* parent);
    ~RowLayout();
    float width() const override;
    float height() const override;

    RowLayout& set_spacing(int spacing);

    void layout();

private:
    int m_spacing = 0;
    float m_content_height = 0.0f;
    float m_content_width = 0.0f;
    void on_update(float dt) override;
};
} // namespace Cubed