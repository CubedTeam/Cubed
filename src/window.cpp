#include "Cubed/window.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/font.hpp"
#include "Cubed/tools/log.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

namespace Cubed {

static int windowed_xpos = 0, windowed_ypos = 0;
static int windowed_width = 1280, windowed_height = 720;

Window::Window(Config& config) : m_config(config) {}

Window::~Window() {
    if (m_imgui_init) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
    }

    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::DestroyContext();
    }

    if (m_window) {
        SDL_GL_DestroyContext(m_context);
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

bool Window::is_mouse_enable() const { return m_mouse_enable; }

const SDL_Window* Window::get_window() const { return m_window; }

SDL_Window* Window::get_window() { return m_window; }

bool Window::handle_event(const Event& e) {
    return std::visit(
        Overloaded{[](const MouseMoveEvent&) { return false; },
                   [this](const MouseButtonEvent& e) {
                       if (handle_mouse_button_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [](const MouseWheelEvent&) { return false; },
                   [this](const KeyEvent& e) {
                       if (handle_key_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [](const TextInputEvent&) { return false; },
                   [this](const WindowResizeEvent& e) {
                       handle_window_resize_event(e);
                       return false;
                   },
                   [](const FrameBufferResizeEvent&) { return false; }

        },
        e);
}

bool Window::handle_key_event(const KeyEvent& e) {
    if (e.key == Key::F11 && e.action == KeyAction::PRESS) {
        toggle_fullscreen();
        return true;
    }
    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
    }
    if (e.key == Key::LEFT_ALT && e.action == KeyAction::PRESS) {
        if (m_game_running) {
            if (m_mouse_enable) {
                disable_mouse();
                set_imgui_enabled(false);
            } else {
                enable_mouse();
                set_imgui_enabled(true);
            }
            return true;
        }
    }

    return false;
}

bool Window::handle_window_resize_event(const WindowResizeEvent& e) {
    m_window_height = e.height;
    m_window_width = e.width;
    return false;
}

bool Window::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.key == MouseKey::LEFT_BUTTON && e.action == KeyAction::PRESS) {
        if (m_game_running) {
            if (m_mouse_enable) {
                disable_mouse();
                set_imgui_enabled(false);
                return true;
            }
        }
    }
    return false;
}

void Window::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        Logger::error("sdl3 init fail");
        exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    m_window_width = m_config.get("window.width", 1280);
    m_window_height = m_config.get("window.height", 720);
    m_window = SDL_CreateWindow("Cubed", m_window_width, m_window_height,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                    SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (m_config.get("window.fullscreen", false)) {
        SDL_SetWindowFullscreen(m_window, true);
    }

    m_context = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        Logger::error("Failed to initialize glad");
        exit(EXIT_FAILURE);
    }
    set_vsync(m_config.get("window.V-Sync", true));

    if (m_game_running) {
        SDL_SetWindowRelativeMouseMode(m_window, true);
    } else {
        SDL_SetWindowRelativeMouseMode(m_window, false);
    }

    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
}

void Window::reload_config() {
    // V-Sync
    set_vsync(m_config.get("window.V-Sync", true));
    // Window
    windowed_width = m_config.get("window.width", 1280);
    windowed_height = m_config.get("window.height", 720);

    set_fullscreen(m_config.get("window.fullscreen", false));

    if (!m_mouse_enable) {
        SDL_SetWindowRelativeMouseMode(m_window, false);
    } else {
        SDL_SetWindowRelativeMouseMode(m_window, true);
    }
    m_config.set("window.width", windowed_width);
    m_config.set("window.height", windowed_height);
}

void Window::toggle_fullscreen() {

    bool is_fullscreen =
        (SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN) != 0;
    if (is_fullscreen) {
        set_fullscreen(false);
    } else {
        set_fullscreen(true);
    }

    m_config.set("window.width", windowed_width);
    m_config.set("window.height", windowed_height);
}
void Window::set_fullscreen(bool full) {
    if (full) {
        SDL_GetWindowPosition(m_window, &windowed_xpos, &windowed_ypos);
        SDL_GetWindowSize(m_window, &windowed_width, &windowed_height);
        SDL_SetWindowFullscreen(m_window, true);
        m_config.set("window.fullscreen", true);
    } else {
        SDL_SetWindowFullscreen(m_window, false);
        SDL_SetWindowPosition(m_window, windowed_xpos, windowed_ypos);
        SDL_SetWindowSize(m_window, windowed_width, windowed_height);

        m_config.set("window.fullscreen", false);
    }
}
void Window::enable_mouse() {
    SDL_WarpMouseInWindow(m_window, width() / 2, height() / 2);
    SDL_SetWindowRelativeMouseMode(m_window, false);
    m_mouse_enable = true;
}
void Window::disable_mouse() {
    SDL_SetWindowRelativeMouseMode(m_window, true);
    if (m_camera) {
        m_camera->reset_camera();
    }
    m_mouse_enable = false;
}

void Window::set_camera(Camera* camera) { m_camera = camera; }
Camera* Window::camera() { return m_camera; }
void Window::set_game_running(bool running) {
    m_game_running = running;
    if (running) {
        disable_mouse();
    } else {
        enable_mouse();
    }
    set_imgui_enabled(false);
}

void Window::should_close_window() {
    SDL_Event quit_event;
    quit_event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit_event);
}

bool Window::is_enable_imgui() const { return m_imgui_enable; }

void Window::set_imgui_enabled(bool enable) { m_imgui_enable = enable; }

void Window::set_vsync(bool enable) {
    if (!SDL_GL_SetSwapInterval(static_cast<int>(enable))) {
        Logger::error("VSync Fail: {}", SDL_GetError());
    }
}
int Window::width() const { return m_window_width; }
int Window::height() const { return m_window_height; }
void Window::imgui_init() {
    float main_scale = SDL_GetWindowDisplayScale(m_window);

    Logger::info("Main Scale {}", main_scale);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NoMouseCursorChange; // Prevent ImGui from
                                              // automatically changing the
                                              // system cursor shape, allowing
                                              // the game to fully control
                                              // cursor appearance (e.g.,
                                              // hidden/disabled custom cursor).
    auto theme = m_config.get("devpanel.theme", 0);
    if (theme == 0) {
        ImGui::StyleColorsDark();
    } else if (theme == 1) {
        ImGui::StyleColorsLight();
    } else {
        ImGui::StyleColorsDark();
        m_config.set("devpanel.theme", 0);
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        main_scale); // Bake a fixed style scale. (until we have a solution for
                     // dynamic style scaling, changing this requires resetting
                     // Style + calling this again)
    style.FontScaleDpi = main_scale;
    style.FontSizeBase = 20.0f;
    ImFont* font = io.Fonts->AddFontFromFileTTF(Font::font_path().c_str());
    ASSERT_MSG(font != nullptr, "Font Load Fail");
    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init("#version 460");

    m_imgui_init = true;
}

} // namespace Cubed