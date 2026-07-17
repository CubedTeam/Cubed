#pragma once

#include "Cubed/argument.hpp"
#include "Cubed/config.hpp"
#include "Cubed/input/event.hpp"

#include <SDL3/SDL.h>
namespace Cubed {
class Camera;
class Renderer;
class Window {
public:
    Window(Config& config);
    ~Window();

    bool is_mouse_enable() const;
    const SDL_Window* get_window() const;
    SDL_Window* get_window();
    void init(const Argument& argument);
    void imgui_init();

    // end of frame to reload!
    bool handle_event(const Event& e);

    void reload_config();

    void toggle_fullscreen();
    void set_fullscreen(bool full);
    void enable_mouse();
    void disable_mouse();

    void set_camera(Camera* camera);
    Camera* camera();

    void set_game_running(bool running);

    void should_close_window();

    bool is_enable_imgui() const;

    void set_imgui_enabled(bool enable);

    void set_vsync(bool enable);

    int width() const;
    int height() const;

private:
    bool m_mouse_enable = true;
    bool m_imgui_init = false;
    bool m_game_running = false;
    SDL_Window* m_window;
    SDL_GLContext m_context;
    int m_window_width = 0;
    int m_window_height = 0;
    Config& m_config;
    Camera* m_camera = nullptr;
    bool m_imgui_enable = false;
    bool handle_key_event(const KeyEvent& e);
    bool handle_window_resize_event(const WindowResizeEvent& e);
    bool handle_mouse_button_event(const MouseButtonEvent& e);
};

} // namespace Cubed