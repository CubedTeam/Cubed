#pragma once
#include "Cubed/input/event.hpp"
#include "glm/ext/vector_float2.hpp"

#include <memory>
#include <vector>
namespace Cubed {
class Renderer;

class Widget {

public:
    Widget(const std::string& id);
    Widget() = default;
    virtual ~Widget() = default;
    virtual void update(float dt);
    virtual void render(Renderer& renderer);
    virtual const std::string& id() const;
    virtual Widget& set_position(const glm::vec2& pos);
    virtual Widget& set_position(float x, float y);
    // Returns the final display size
    virtual float width() const;
    virtual float height() const;
    virtual const glm::vec2& pos() const;

    virtual bool handle_key_event(const KeyEvent& e);
    virtual bool handle_mouse_button_event(const MouseButtonEvent& e);
    virtual bool handle_mouse_wheel_event(const MouseWheelEvent& e);
    virtual bool handle_window_resize_event(const WindowResizeEvent& e);
    virtual bool handle_mouse_move_event(const MouseMoveEvent& e);

    template <typename T, typename... Args> T& add_child(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *widget;
        m_children.emplace_back(std::move(widget));
        return ref;
    };

protected:
    virtual void on_update(float dt);
    virtual void on_render(Renderer& renderer);
    std::string m_id;
    // Center is at the top-left corner, position is at the top-left corner
    glm::vec2 m_pos{0.0f, 0.0f};

private:
    Widget* m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;
};
} // namespace Cubed