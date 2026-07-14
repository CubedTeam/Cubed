#pragma once

#include "Cubed/ui/widget.hpp"

#include <memory>
namespace Cubed {
class UIManager {
public:
    UIManager() = default;
    UIManager(const UIManager&) = delete;
    UIManager(UIManager&&) = delete;
    UIManager& operator=(const UIManager&) = delete;
    UIManager& operator=(UIManager&&) = delete;
    virtual ~UIManager();

    virtual void init();

    virtual void update(float dt);
    virtual void render(Renderer& renderer);
    virtual bool handle_event(const Event& e);

    virtual Widget* get_widget(std::string name);

    virtual bool handle_mouse_move_event(const MouseMoveEvent& e);
    virtual bool handle_mouse_button_event(const MouseButtonEvent& e);
    virtual bool handle_window_resize_event(const WindowResizeEvent& e);
    virtual bool handle_mouse_wheel_event(const MouseWheelEvent& e);
    virtual bool handle_key_event(const KeyEvent& e);

protected:
    std::unique_ptr<Widget> m_root_widget;
    std::unordered_map<std::string, Widget*> m_widgets;
};
} // namespace Cubed