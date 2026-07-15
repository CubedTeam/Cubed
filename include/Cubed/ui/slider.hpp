#pragma once

#include "Cubed/ui/image.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/widget.hpp"

namespace Cubed {

enum class ValueType {
    FLOAT,
    INT,
};

class Slider : public Widget {
public:
    Slider(Widget* parent);

    Slider& set_slider(float* value, const float& min, const float& max);
    Slider& set_slider(int* value, const int& min, const int& max);
    float width() const override;
    float height() const override;

    Slider& set_scale(float scale);
    Slider& set_width(float width);
    Slider& set_height(float h);
    Slider& set_text(const std::string& text);
    Slider& set_track_image(const std::string& path,
                            TextureManager& texture_manager);
    Slider& set_thumb_image(const std::string& path,
                            TextureManager& texture_manager);

    Slider& set_default_image(TextureManager& texture_manager);

    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;

private:
    static constexpr float PADDING = 5.0f;
    static constexpr float DEFAULT_SCALE = 3.0f;
    static constexpr const char* DEFAULT_TRACK_IMAGE =
        "texture/ui/slider_track001.png";
    static constexpr const char* DEFAULT_THUMB_IMAGE =
        "texture/ui/slider_thumb001.png";
    std::unique_ptr<Image> m_track;
    std::unique_ptr<Image> m_thumb;
    std::unique_ptr<Label> m_label;
    ValueType m_type = ValueType::FLOAT;
    float m_scale = DEFAULT_SCALE;
    float m_width = NORMAL_SLIDER_WIDTH;
    float m_height = NORMAL_SLIDER_HEIGHT;
    float m_xpos = 0.0f;
    bool m_dragging = false;
    bool m_inside = false;
    float* m_float_value = nullptr;
    int* m_int_value = nullptr;
    float m_min = 0;
    float m_max = 0;
    std::string m_text;

    void init_track();
    void init_thumb();

    void update_value(float mouse_x);

    void on_update(float) override;

    void on_render(Renderer& renderer) override;
    void update_text_scale();
};
} // namespace Cubed