#include "Cubed/window.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/font.hpp"
#include "Cubed/tools/log.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Cubed {

static int windowed_xpos = 0, windowed_ypos = 0;
static int windowed_width = 800, windowed_height = 600;

Window::Window(Config& config) : m_config(config) {}

Window::~Window() {
    if (m_imgui_init) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }

    if (ImGui::GetCurrentContext() != nullptr) {
        ImGui::DestroyContext();
    }

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
}

bool Window::is_mouse_enable() const { return m_mouse_enable; }

const GLFWwindow* Window::get_glfw_window() const { return m_window; }

GLFWwindow* Window::get_glfw_window() { return m_window; }

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
    if (!glfwInit()) {
        Logger::error("glfw init fail");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window_width = m_config.get("window.width", 800);
    m_window_height = m_config.get("window.height", 600);

    if (m_config.get("window.fullscreen", false)) {
        GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
        m_window = glfwCreateWindow(mode->width, mode->height, "Cubed",
                                    primary_monitor, NULL);
    } else {
        m_window = glfwCreateWindow(m_window_width, m_window_height, "Cubed",
                                    NULL, NULL);
    }

    glfwMakeContextCurrent(m_window);
    if (m_config.get("window.V-Sync", true)) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    if (m_game_running) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    } else {
        Logger::warn("Don,t support Raw Mouse Motion");
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    glfwSetWindowPos(m_window, static_cast<int>(mode->width / 2.0f) - 400,
                     static_cast<int>(mode->height / 2.0f) - 300);
}

void Window::reload_config() {
    // V-Sync
    if (m_config.get("window.V-Sync", true)) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }
    // Window
    windowed_width = m_config.get("window.width", 800);
    windowed_height = m_config.get("window.height", 600);

    if (m_config.get("window.fullscreen", false)) {
        glfwGetWindowPos(m_window, &windowed_xpos, &windowed_ypos);
        glfwGetWindowSize(m_window, &windowed_width, &windowed_height);

        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primary);

        glfwSetWindowMonitor(m_window, primary, 0, 0, mode->width, mode->height,
                             GL_DONT_CARE);
    } else {
        GLFWmonitor* monitor = glfwGetWindowMonitor(m_window);
        if (monitor != nullptr) {
            glfwSetWindowMonitor(m_window, nullptr, windowed_xpos,
                                 windowed_ypos, windowed_width, windowed_height,
                                 0);
        } else {
            Logger::error("Can't Find Monitor");
        }
    }
    if (!m_mouse_enable) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    m_config.set("window.width", windowed_width);
    m_config.set("window.height", windowed_height);
}

void Window::toggle_fullscreen() {

    GLFWmonitor* monitor = glfwGetWindowMonitor(m_window);
    if (monitor != nullptr) {
        glfwSetWindowMonitor(m_window, nullptr, windowed_xpos, windowed_ypos,
                             windowed_width, windowed_height, 0);

        m_config.set("window.fullscreen", false);
    } else {
        glfwGetWindowPos(m_window, &windowed_xpos, &windowed_ypos);
        glfwGetWindowSize(m_window, &windowed_width, &windowed_height);

        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primary);

        glfwSetWindowMonitor(m_window, primary, 0, 0, mode->width, mode->height,
                             GL_DONT_CARE);
        m_config.set("window.fullscreen", true);
    }
    m_config.set("window.width", windowed_width);
    m_config.set("window.height", windowed_height);
}

void Window::enable_mouse() {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_mouse_enable = true;
}
void Window::disable_mouse() {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

void Window::should_close_window() { glfwSetWindowShouldClose(m_window, true); }

bool Window::is_enable_imgui() const { return m_imgui_enable; }

void Window::set_imgui_enabled(bool enable) { m_imgui_enable = enable; }

void Window::imgui_init() {
    float dpi_scale_x, dpi_scale_y;
    glfwGetWindowContentScale(m_window, &dpi_scale_x, &dpi_scale_y);
    // float main_scale =
    // ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    float main_scale = dpi_scale_x;
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
    ImGui_ImplGlfw_InitForOpenGL(m_window, false);
    ImGui_ImplOpenGL3_Init();

    m_imgui_init = true;
}

} // namespace Cubed