#pragma once
#include "Cubed/input/event.hpp"
namespace Cubed {
class Renderer;

enum class SceneType { MAIN_MENU, WORLD, CREDITS, SETTINGS };

class Scene {
public:
    Scene() = default;

    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

    virtual ~Scene() = default;
    virtual void update(float dt) = 0;
    virtual void render(Renderer& renderer) = 0;
    virtual bool handle_event(const Event& e) = 0;
    virtual void on_enter() {};
    virtual void on_leave() {};
    virtual void on_re_enter() {};

protected:
    virtual bool handle_mouse_move_event(const MouseMoveEvent&) {
        return false;
    }
    virtual bool handle_mouse_button_event(const MouseButtonEvent&) {
        return false;
    }
    virtual bool handle_window_resize_event(const WindowResizeEvent&) {
        return false;
    }
    virtual bool handle_mouse_wheel_event(const MouseWheelEvent&) {
        return false;
    }
    virtual bool handle_key_event(const KeyEvent&) { return false; }
};
} // namespace Cubed