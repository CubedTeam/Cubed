#pragma once
#include "Cubed/argument.hpp"
#include "Cubed/audio/audio_engine.hpp"
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
    void handle_mouse_move(float xpos, float ypos, float xrel, float yrel);
    void handle_sdl_key(SDL_Event& e);
    void handle_sdl_mouse_button(SDL_Event& e);
    void handle_window_focus(bool focused);
    void handle_window_resize(int new_width, int new_height);
    void handle_framebuffer_resize(int new_width, int new_height);
    void handle_mouse_scroll(float xoffset, float yoffset);
    void handle_text_input(const char* text);

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

    std::string get_clipboard_text();

    void start_text_input();
    void stop_text_input();
    void update_text_input_area(const glm::vec4& textbox,
                                float cursor_position_x);

private:
    Config m_game_config;

    Window m_window;

    TextureManager m_texture_manager;

    AudioEngine m_audio;

    Renderer m_renderer;

    SceneManager m_scene_manager;

    inline static uint64_t last_tick = 0;
    inline static uint64_t current_tick = 0;
    inline static float dt = 0.0f;
    inline static float fps_time_count = 0.0f;
    inline static int frame_count = 0;
    inline static int fps = 0;
    Argument m_argument;
    bool m_running = true;
    bool m_opengl_init = false;
    void init(int argc, char** argv);
    void handle_argument(int argc, char** argv);
    auto init_camera();
    auto init_texture();
    auto init_world();

    void render();
    void run();
    void update();

    void handle_sdl_event(SDL_Event& e);

    void dispatch_event(const Event& e);
};

} // namespace Cubed