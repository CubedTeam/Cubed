#pragma once
#include "Cubed/audio/audio_engine.hpp"
#define GLFW_INCLUDE_NONE
#include "Cubed/argument.hpp"
#include "Cubed/config.hpp"
#include "Cubed/dev_panel.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/texture_manager.hpp"
#include "Cubed/window.hpp"
namespace Cubed {

class App {
public:
    App();
    ~App();
    static void cursor_position_callback(GLFWwindow* window, double xpos,
                                         double ypos);
    static void key_callback(GLFWwindow* window, int key, int scancode,
                             int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button,
                                      int action, int mods);
    static void window_focus_callback(GLFWwindow* window, int focused);
    static void window_reshape_callback(GLFWwindow* window, int new_width,
                                        int new_height);
    static void framebuffer_size_callback(GLFWwindow* window, int new_width,
                                          int new_height);
    static void mouse_scroll_callback(GLFWwindow* window, double xoffset,
                                      double yoffset);
    static void cursor_enter_callback(GLFWwindow* window, int entered);
    static void char_callback(GLFWwindow* window, unsigned int ch);
    static int start_cubed_application(int argc, char** argv);

    static unsigned int seed();
    static float delta_time();
    static float get_fps();

    Renderer& renderer();
    TextureManager& texture_manager();
    Window& window();

    Config& config();
    const Argument& argument() const;
    AudioEngine& audio();

    const char* get_clipboard_text();

private:
    Config m_game_config;

    TextureManager m_texture_manager;

    AudioEngine m_audio;

    Renderer m_renderer;

    Window m_window;

    SceneManager m_scene_manager;

    inline static double last_time = glfwGetTime();
    inline static double current_time = glfwGetTime();
    inline static double dt = 0.0f;
    inline static double fps_time_count = 0.0f;
    inline static int frame_count = 0;
    inline static int fps = 0;
    Argument m_argument;

    void init(int argc, char** argv);
    void handle_argument(int argc, char** argv);
    void handle_toml();
    auto init_camera();
    auto init_texture();
    auto init_world();

    void render();
    void run();
    void update();

    void dispatch_event(const Event& e);
};

} // namespace Cubed