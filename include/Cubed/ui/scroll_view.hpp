#pragma once

#include "Cubed/ui/widget.hpp"
namespace Cubed {
class ColumnLayout;
class ScrollView : public Widget {
public:
    ScrollView(const ScrollView&) = delete;
    ScrollView(ScrollView&&) = delete;
    ScrollView& operator=(const ScrollView&) = delete;
    ScrollView& operator=(ScrollView&&) = delete;

    ScrollView(Widget* parent);
    ~ScrollView();

    void render(Renderer& renderer) override;

    float width() const override;
    // visable height, you need notice offset
    float height() const override;

    ScrollView& set_child(std::unique_ptr<ColumnLayout> children);

    bool handle_mouse_wheel_event(const MouseWheelEvent& e) override;
    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    Widget& set_fill_parent(bool fill) override;
    Widget& set_fill_width(bool fill) override;
    Widget& set_fill_height(bool fill) override;

private:
    ColumnLayout* m_child = nullptr;
    bool m_hovered = false;
    float m_scroll_speed = 50.0f;
};
} // namespace Cubed