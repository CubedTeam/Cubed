#pragma once
#include "Cubed/input/event.hpp"
#include "Cubed/ui/anchor.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
namespace Cubed {
class Renderer;

constexpr float NORMAL_BUTTON_WIDTH = 225.0f;
constexpr float NORMAL_BUTTON_HEIGHT = 20.0f;
constexpr float NORMAL_SLIDER_WIDTH = 225.0f;
constexpr float NORMAL_SLIDER_HEIGHT = 20.0f;
constexpr float NORMAL_TEXTFIELD_WIDTH = 225.0f;
constexpr float NORMAL_TEXTFIELD_HEIGHT = 20.0f;

enum class TraversalOrder { FRONT_TO_BACK, BACK_TO_FRONT };

class Widget {

public:
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;

    Widget(Widget* parent);
    virtual ~Widget() = default;
    virtual void update(float dt);
    virtual void render(Renderer& renderer);
    virtual Widget& set_anchor(Anchor anchor);
    virtual Widget& set_offset(glm::ivec2 offset);
    virtual Widget& set_window_size(int width, int height);
    virtual Widget& set_visible(bool visible);
    // Returns the final display size
    virtual float width() const;
    virtual float height() const;
    virtual glm::vec2 pos() const;

    virtual bool handle_key_event(const KeyEvent& e);
    virtual bool handle_mouse_button_event(const MouseButtonEvent& e);
    virtual bool handle_mouse_wheel_event(const MouseWheelEvent& e);
    virtual bool handle_window_resize_event(const WindowResizeEvent& e);
    virtual bool handle_mouse_move_event(const MouseMoveEvent& e);
    virtual bool handle_text_input_event(const TextInputEvent& e);

    void set_order(TraversalOrder order);

    template <typename T, typename... Args> T& add_child(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)..., this);
        T& ref = *widget;
        m_children.emplace_back(std::move(widget));
        return ref;
    };

protected:
    Widget* m_parent = nullptr;
    float m_window_height = 0;
    float m_window_width = 0;
    // Center is at the top-left corner, position is at the top-left corner
    Anchor m_anchor = Anchor::TOP_LEFT;
    bool m_visible = true;

    glm::ivec2 m_offset{0, 0};

    std::vector<std::unique_ptr<Widget>>& children();

    const std::vector<std::unique_ptr<Widget>>& children() const;

    virtual void on_update(float dt);
    virtual void on_render(Renderer& renderer);
    virtual glm::vec2 compute_position() const;

private:
    std::vector<std::unique_ptr<Widget>> m_children;
    TraversalOrder m_order = TraversalOrder::BACK_TO_FRONT;
};
} // namespace Cubed