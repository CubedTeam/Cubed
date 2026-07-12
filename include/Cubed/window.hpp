#pragma once

#include "Cubed/config.hpp"
#include "Cubed/input/event.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
namespace Cubed {
class Camera;
class Renderer;
class Window {
public:
    Window(Config& config);
    ~Window();

    bool is_mouse_enable() const;
    const GLFWwindow* get_glfw_window() const;
    GLFWwindow* get_glfw_window();
    void init();
    void imgui_init();

    // end of frame to reload!
    bool handle_event(const Event& e);

    void hot_reload();

    void toggle_fullscreen();
    void toggle_mouse_able();

    void set_camera(Camera* camera);
    Camera* camera();

private:
    bool m_mouse_enable = false;
    bool m_imgui_init = false;
    GLFWwindow* m_window;
    int m_window_width;
    int m_window_height;
    Config& m_config;
    Camera* m_camera = nullptr;
    bool handle_key_event(const KeyEvent& e);
    bool handle_window_resize_event(const WindowResizeEvent& e);
    bool handle_mouse_button_event(const MouseButtonEvent& e);
};

} // namespace Cubed