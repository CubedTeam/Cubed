#pragma once

#include "Cubed/ui/image.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/rect.hpp"
#include "Cubed/ui/widget.hpp"
namespace Cubed {
class App;
class TextField : public Widget {
public:
    TextField(const TextField&) = delete;
    TextField(TextField&&) = delete;
    TextField& operator=(const TextField&) = delete;
    TextField& operator=(TextField&&) = delete;
    TextField(Widget* parent);

    TextField& set_scale(float scale);

    float width() const override;
    float height() const override;

    TextField& set_width(float width);
    TextField& set_height(float height);
    TextField& set_show_text(const std::string& text);
    TextField& set_background_image(const std::string& path,
                                    TextureManager& texture_manager);
    TextField& set_default_image(TextureManager& texture_manager);
    TextField& set_auto_scale(bool auto_scale);
    TextField& set_app(App* app);
    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;
    bool handle_text_input_event(const TextInputEvent& e) override;
    bool handle_key_event(const KeyEvent& e) override;
    bool handle_window_resize_event(const WindowResizeEvent& e) override;
    const std::string& input_text() const;

    template <typename F> TextField& set_on_finish(F&& f) {
        m_on_finished = std::forward<F>(f);
        return *this;
    }

private:
    static constexpr float PADDING = 5.0f;
    static constexpr float DEFAULT_SCALE = 3.0f;
    static constexpr float TEXT_SCALE = 0.6f;
    static constexpr float CURSOR_INTERVAL = 0.5f;
    static constexpr const char* DEFAULT_TEXT_FIELD_IMAGE =
        "texture/ui/textfield001.png";

    App* m_app;
    std::unique_ptr<Image> m_background;
    std::unique_ptr<Label> m_foreground;
    std::unique_ptr<Rect> m_cursor;
    float m_cursor_timer = 0.0f;
    bool m_cursor_visible = true;
    bool m_inside = false;
    bool m_typing = false;
    bool m_auto_scale = false;
    bool m_ctrl_press = false;
    float m_width = NORMAL_TEXTFIELD_WIDTH;
    float m_height = NORMAL_TEXTFIELD_HEIGHT;
    float m_scale = DEFAULT_SCALE;

    std::string m_input_text;
    std::string m_show_text;

    std::function<void()> m_on_finished;
    void update_text_scale();
    void on_render(Renderer& renderer) override;
    void on_update(float dt) override;
    void update_show_text();
    void update_input_area();
};
} // namespace Cubed