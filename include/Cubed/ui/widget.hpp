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
class Widget {

public:
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget& operator=(Widget&&) = delete;

    Widget(const std::string& id, Widget* parent);
    Widget(Widget* parent);
    virtual ~Widget() = default;
    virtual void update(float dt);
    virtual void render(Renderer& renderer);
    virtual const std::string& id() const;
    virtual Widget& set_anchor(Anchor anchor);
    virtual Widget& set_offset(glm::ivec2 offset);
    virtual void set_window_size(int width, int height);
    // Returns the final display size
    virtual float width() const = 0;
    virtual float height() const = 0;
    virtual glm::vec2 pos() const;

    virtual bool handle_key_event(const KeyEvent& e);
    virtual bool handle_mouse_button_event(const MouseButtonEvent& e);
    virtual bool handle_mouse_wheel_event(const MouseWheelEvent& e);
    virtual bool handle_window_resize_event(const WindowResizeEvent& e);
    virtual bool handle_mouse_move_event(const MouseMoveEvent& e);
    virtual bool handle_text_input_event(const TextInputEvent& e);
    template <typename T, typename... Args> T& add_child(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)..., this);
        T& ref = *widget;
        m_children.emplace_back(std::move(widget));
        return ref;
    };

protected:
    Widget* m_parent = nullptr;
    std::string m_id;
    float m_window_height = 0;
    float m_window_width = 0;
    // Center is at the top-left corner, position is at the top-left corner
    Anchor m_anchor = Anchor::TOP_LEFT;
    glm::ivec2 m_offset{0, 0};

    std::vector<std::unique_ptr<Widget>>& children();

    const std::vector<std::unique_ptr<Widget>>& children() const;

    virtual void on_update(float dt);
    virtual void on_render(Renderer& renderer);
    virtual glm::vec2 compute_position() const;

private:
    std::vector<std::unique_ptr<Widget>> m_children;
};
} // namespace Cubed