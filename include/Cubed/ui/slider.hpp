#pragma once

#include "Cubed/ui/image.hpp"
#include "Cubed/ui/widget.hpp"

namespace Cubed {
class Slider : public Widget {
public:
    Slider(Widget* parent);

    Slider& set_slider(float* value, const float& min, const float& max);

    Image& get_track();
    Image& get_thumb();

    float width() const override;
    float height() const override;

    Slider& set_scale(float scale);
    Slider& set_width(float width);
    Slider& set_height(float h);

    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;

private:
    static constexpr float THUMB_WIDTH = 5.0f;

    void init_track();
    void init_thumb();

    void update_value(float mouse_x);

    void on_update(float) override;

    void on_render(Renderer& renderer) override;

    std::unique_ptr<Image> m_track;
    std::unique_ptr<Image> m_thumb;
    float m_scale = 1.0f;
    float m_width = NORMAL_SLIDER_WIDTH;
    float m_height = NORMAL_SLIDER_HEIGHT;
    float m_xpos = 0.0f;
    bool m_dragging = false;
    bool m_inside = false;
    float* m_value = nullptr;
    float m_min = 0;
    float m_max = 0;
};
} // namespace Cubed