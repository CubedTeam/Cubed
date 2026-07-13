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

    void render(Renderer& renderer) override;
    void update(float dt) override;

    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;
    void set_window_size(int width, int height) override;
    Button& set_scale(float scale);
    Widget& set_anchor(Anchor anchor) override;
    Widget& set_offset(glm::ivec2 offset) override;
    glm::vec2 pos() const override;
    float width() const override;
    float height() const override;
    float scale() const;
    template <typename F> Button& set_clicked(F&& f) {
        m_clicked = std::forward<F>(f);
        return *this;
    }

    template <typename T, typename... Args> T& set_background(Args&&... args) {
        auto w = std::make_unique<T>(std::forward<Args>(args)..., m_parent);
        T& ref = *w;
        m_background = std::move(w);
        return ref;
    }

    template <typename T, typename... Args> T& set_foreground(Args&&... args) {
        auto w = std::make_unique<T>(std::forward<Args>(args)..., this);
        T& ref = *w;
        m_foreground = std::move(w);
        m_foreground->set_anchor(Anchor::CENTER);
        return ref;
    }

private:
    std::function<void()> m_clicked;
    std::unique_ptr<Image> m_background;
    std::unique_ptr<Label> m_foreground;
    bool m_hovered = false;
};
} // namespace Cubed