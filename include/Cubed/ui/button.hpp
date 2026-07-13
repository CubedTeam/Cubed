#pragma once

#include "Cubed/ui/widget.hpp"

#include <functional>

namespace Cubed {
class Button : public Widget {
public:
    Button();

    Button(const Button&) = delete;
    Button(Button&&) = delete;
    Button& operator=(const Button&) = delete;
    Button& operator=(Button&&) = delete;

    void render(Renderer& renderer) override;
    void update(float dt) override;

    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;

    Widget& set_position(const glm::vec2& pos) override;
    Widget& set_position(float x, float y) override;

    float width() const override;
    float height() const override;

    template <typename F> Button& set_clicked(F&& f) {
        m_clicked = std::forward<F>(f);
        return *this;
    }

    template <typename T, typename... Args> T& set_background(Args&&... args) {
        auto w = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *w;
        m_background = std::move(w);
        return ref;
    }

    template <typename T, typename... Args> T& set_foreground(Args&&... args) {
        auto w = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *w;
        m_foreground = std::move(w);
        return ref;
    }

private:
    std::function<void()> m_clicked;
    std::unique_ptr<Widget> m_background;
    std::unique_ptr<Widget> m_foreground;
    bool m_hovered = false;
    glm::vec2 m_min_pos;
    glm::vec2 m_max_pos;
};
} // namespace Cubed