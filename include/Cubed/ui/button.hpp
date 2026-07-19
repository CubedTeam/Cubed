#pragma once

#include "Cubed/ui/image.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/widget.hpp"

#include <functional>

namespace Cubed {
class Button : public Widget {
public:
    Button(Widget* parent);

    Button(const Button&) = delete;
    Button(Button&&) = delete;
    Button& operator=(const Button&) = delete;
    Button& operator=(Button&&) = delete;

    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    virtual bool handle_mouse_button_event(const MouseButtonEvent& e) override;
    Button& set_scale(float scale);

    float width() const override;
    float height() const override;

    Button& set_width(float width) override;
    Button& set_height(float height) override;
    Button& set_background_image(const std::string& path,
                                 TextureManager& texture_manager);
    Button& set_default_image(TextureManager& texture_manager);
    virtual Button& set_text(const std::string& text);

    Button& set_auto_scale(bool auto_scale);
    Button& set_enable(bool enable);
    float scale() const;
    template <typename F> Button& set_clicked(F&& f) {
        m_clicked = std::forward<F>(f);
        return *this;
    }

protected:
    std::unique_ptr<Image> m_background;
    std::unique_ptr<Label> m_foreground;
    bool m_hovered = false;
    bool m_enable = true;
    void update_text_scale();

private:
    static constexpr float PADDING = 5.0f;
    static constexpr float DEFAULT_SCALE = 3.0f;
    static constexpr float TEXT_SCALE = 0.6f;

    static constexpr const char* DEFAULT_BUTTON_IMAGE =
        "texture/ui/button001.png";
    std::function<void()> m_clicked;

    float m_scale = DEFAULT_SCALE;
    bool m_auto_scale = false;
    void on_render(Renderer& renderer) override;
    void on_update(float dt) override;
};
} // namespace Cubed