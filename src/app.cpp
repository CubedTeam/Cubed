#include "Cubed/app.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/config.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/tools/arg_parser.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/system_info.hpp"
#include "version.hpp"

#include <exception>
#include <imgui_impl_glfw.h>
namespace Cubed {

App::App()

    : m_game_config(ASSETS_PATH "config.toml"),
      m_texture_manager(m_game_config), m_audio(m_game_config),
      m_renderer(m_texture_manager, m_game_config), m_window(m_game_config),
      m_scene_manager(*this) {}

App::~App() {}
void App::cursor_position_callback(GLFWwindow* window, double xpos,
                                   double ypos) {
    ImGuiIO& io = ImGui::GetIO();

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    ASSERT_MSG(app, "nullptr");
    if (io.WantCaptureMouse && app->m_window.is_mouse_enable()) {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
        return;
    }
    if (!app->m_window.is_mouse_enable()) {
        app->m_scene_manager.handle_event(
            MouseMoveEvent{static_cast<float>(xpos), static_cast<float>(ypos)});
    }
}
void App::init(int argc, char** argv) {
    handle_toml();
    handle_argument(argc, argv);

    m_window.init();
    m_window.imgui_init();

    Logger::info("Window Init Success");

    glfwSetWindowUserPointer(m_window.get_glfw_window(), this);

    glfwSetCursorPosCallback(m_window.get_glfw_window(),
                             cursor_position_callback);
    glfwSetMouseButtonCallback(m_window.get_glfw_window(),
                               mouse_button_callback);
    glfwSetWindowFocusCallback(m_window.get_glfw_window(),
                               window_focus_callback);
    glfwSetWindowSizeCallback(m_window.get_glfw_window(),
                              window_reshape_callback);
    glfwSetFramebufferSizeCallback(m_window.get_glfw_window(),
                                   framebuffer_size_callback);
    glfwSetKeyCallback(m_window.get_glfw_window(), key_callback);
    glfwSetScrollCallback(m_window.get_glfw_window(), mouse_scroll_callback);
    glfwSetCursorEnterCallback(m_window.get_glfw_window(),
                               cursor_enter_callback);
    glfwSetCharCallback(m_window.get_glfw_window(), char_callback);

    m_audio.init();

    ChunkGenerator::init();
    BlockManager::init();
    m_renderer.init(m_argument.debug_on);
    Logger::info("Renderer Init Success");
    // MapTable::init_map();
    m_texture_manager.init_texture();
    Logger::info("Texture Load Success");
    if (!m_argument.is_client) {
        m_server.start_server(m_argument.port);
    }

    m_scene_manager.request_push(SceneType::MAIN_MENU);

    {
        int w, h;
        glfwGetWindowSize(m_window.get_glfw_window(), &w, &h);
        window_reshape_callback(m_window.get_glfw_window(), w, h);
    }

    {
        int w, h;
        glfwGetFramebufferSize(m_window.get_glfw_window(), &w, &h);
        framebuffer_size_callback(m_window.get_glfw_window(), w, h);
    }
}

void App::handle_argument(int argc, char** argv) {

    static const std::unordered_map<std::string_view,
                                    std::function<void(ArgParser&)>>
        HANDLERS{

            {"--client", [&](ArgParser&) { m_argument.is_client = true; }},

            {"--host", [&](ArgParser&) { m_argument.is_client = false; }},

            {"-p",
             [&](ArgParser& p) {
                 auto arg = p.require_next("-p");

                 auto r = std::from_chars(arg.data(), arg.data() + arg.size(),
                                          m_argument.port);

                 if (r.ec != std::errc{} || r.ptr != arg.data() + arg.size()) {
                     throw std::runtime_error(
                         std::format("Invalid port: {}", arg));
                 }

                 if (m_argument.port > 65535) {
                     throw std::runtime_error(
                         std::format("Port {} out of range", m_argument.port));
                 }
             }},

            {"--ip",
             [&](ArgParser& p) {
                 auto arg = p.require_next("--ip");
                 m_argument.ip = arg;
             }},
            {"--player",
             [&](ArgParser& p) {
                 auto arg = p.require_next("--player");
                 m_argument.player = arg;
             }},
            {"-V",
             [&](ArgParser) {
                 std::cout << CUBED_VERSION << "\n";
                 exit(EXIT_SUCCESS);
             }},

            {"--no-debug",
             [&](ArgParser) {
                 m_argument.debug_on = false;
                 Logger::info("Switch off opengl debug out put");
             }}

        };
    ArgParser parser(argc, argv);

    while (parser.has_next()) {
        auto arg = parser.next();
        if (auto it = HANDLERS.find(arg); it != HANDLERS.end()) {
            it->second(parser);
        } else {
            Logger::warn("Unknown argument: {}", arg);
        }
    }
}

void App::handle_toml() {
    toml::table server;
    try {
        server = toml::parse_file("server.toml");
    } catch (const toml::parse_error& e) {
        // Logger::warn("Ip toml parse error {}", e.what());
        return;
    }

    m_argument.ip =
        *TOML::safe_get_value(server, "ip", std::string("127.0.01"));
    m_argument.port = *TOML::safe_get_value(server, "port", 25530);
    m_argument.is_client = *TOML::safe_get_value(server, "client", false);
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action,
                       int mods) {
    ImGuiIO& io = ImGui::GetIO();

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");
    // ImGui_ImplGlfw_CursorEnterCallback(window,
    // !app->m_window.is_mouse_enable());
    if (io.WantCaptureKeyboard && app->m_window.is_mouse_enable()) {
        if ((key == GLFW_KEY_LEFT_ALT) && action == GLFW_PRESS) {
            app->dispatch_event(KeyEvent{Key::LEFT_ALT, KeyAction::PRESS});
            return;
        }
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        return;
    }
    Key pkey;
    KeyAction act;
    switch (key) {
    // Letter keys
    case GLFW_KEY_A:
        pkey = Key::A;
        break;
    case GLFW_KEY_B:
        pkey = Key::B;
        break;
    case GLFW_KEY_C:
        pkey = Key::C;
        break;
    case GLFW_KEY_D:
        pkey = Key::D;
        break;
    case GLFW_KEY_E:
        pkey = Key::E;
        break;
    case GLFW_KEY_F:
        pkey = Key::F;
        break;
    case GLFW_KEY_G:
        pkey = Key::G;
        break;
    case GLFW_KEY_H:
        pkey = Key::H;
        break;
    case GLFW_KEY_I:
        pkey = Key::I;
        break;
    case GLFW_KEY_J:
        pkey = Key::J;
        break;
    case GLFW_KEY_K:
        pkey = Key::K;
        break;
    case GLFW_KEY_L:
        pkey = Key::L;
        break;
    case GLFW_KEY_M:
        pkey = Key::M;
        break;
    case GLFW_KEY_N:
        pkey = Key::N;
        break;
    case GLFW_KEY_O:
        pkey = Key::O;
        break;
    case GLFW_KEY_P:
        pkey = Key::P;
        break;
    case GLFW_KEY_Q:
        pkey = Key::Q;
        break;
    case GLFW_KEY_R:
        pkey = Key::R;
        break;
    case GLFW_KEY_S:
        pkey = Key::S;
        break;
    case GLFW_KEY_T:
        pkey = Key::T;
        break;
    case GLFW_KEY_U:
        pkey = Key::U;
        break;
    case GLFW_KEY_V:
        pkey = Key::V;
        break;
    case GLFW_KEY_W:
        pkey = Key::W;
        break;
    case GLFW_KEY_X:
        pkey = Key::X;
        break;
    case GLFW_KEY_Y:
        pkey = Key::Y;
        break;
    case GLFW_KEY_Z:
        pkey = Key::Z;
        break;

    // Digit keys (main keyboard area)
    case GLFW_KEY_0:
        pkey = Key::DIGIT_0;
        break;
    case GLFW_KEY_1:
        pkey = Key::DIGIT_1;
        break;
    case GLFW_KEY_2:
        pkey = Key::DIGIT_2;
        break;
    case GLFW_KEY_3:
        pkey = Key::DIGIT_3;
        break;
    case GLFW_KEY_4:
        pkey = Key::DIGIT_4;
        break;
    case GLFW_KEY_5:
        pkey = Key::DIGIT_5;
        break;
    case GLFW_KEY_6:
        pkey = Key::DIGIT_6;
        break;
    case GLFW_KEY_7:
        pkey = Key::DIGIT_7;
        break;
    case GLFW_KEY_8:
        pkey = Key::DIGIT_8;
        break;
    case GLFW_KEY_9:
        pkey = Key::DIGIT_9;
        break;

    // Function keys
    case GLFW_KEY_F1:
        pkey = Key::F1;
        break;
    case GLFW_KEY_F2:
        pkey = Key::F2;
        break;
    case GLFW_KEY_F3:
        pkey = Key::F3;
        break;
    case GLFW_KEY_F4:
        pkey = Key::F4;
        break;
    case GLFW_KEY_F5:
        pkey = Key::F5;
        break;
    case GLFW_KEY_F6:
        pkey = Key::F6;
        break;
    case GLFW_KEY_F7:
        pkey = Key::F7;
        break;
    case GLFW_KEY_F8:
        pkey = Key::F8;
        break;
    case GLFW_KEY_F9:
        pkey = Key::F9;
        break;
    case GLFW_KEY_F10:
        pkey = Key::F10;
        break;
    case GLFW_KEY_F11:
        pkey = Key::F11;
        break;
    case GLFW_KEY_F12:
        pkey = Key::F12;
        break;

    // Control keys
    case GLFW_KEY_BACKSPACE:
        pkey = Key::BACKSPACE;
        break;
    case GLFW_KEY_TAB:
        pkey = Key::TAB;
        break;
    case GLFW_KEY_ENTER:
        pkey = Key::ENTER;
        break;
    case GLFW_KEY_ESCAPE:
        pkey = Key::ESCAPE;
        break;
    case GLFW_KEY_SPACE:
        pkey = Key::SPACE;
        break;
    case GLFW_KEY_CAPS_LOCK:
        pkey = Key::CAPS_LOCK;
        break;
    case GLFW_KEY_NUM_LOCK:
        pkey = Key::NUM_LOCK;
        break;
    case GLFW_KEY_SCROLL_LOCK:
        pkey = Key::SCROLL_LOCK;
        break;

    // Modifier keys
    case GLFW_KEY_LEFT_SHIFT:
        pkey = Key::LEFT_SHIFT;
        break;
    case GLFW_KEY_RIGHT_SHIFT:
        pkey = Key::RIGHT_SHIFT;
        break;
    case GLFW_KEY_LEFT_CONTROL:
        pkey = Key::LEFT_CTRL;
        break;
    case GLFW_KEY_RIGHT_CONTROL:
        pkey = Key::RIGHT_CTRL;
        break;
    case GLFW_KEY_LEFT_ALT:
        pkey = Key::LEFT_ALT;
        break;
    case GLFW_KEY_RIGHT_ALT:
        pkey = Key::RIGHT_ALT;
        break;
    case GLFW_KEY_LEFT_SUPER:
        pkey = Key::LEFT_SUPER;
        break;
    case GLFW_KEY_RIGHT_SUPER:
        pkey = Key::RIGHT_SUPER;
        break;

    // Navigation keys
    case GLFW_KEY_INSERT:
        pkey = Key::INSERT;
        break;
    case GLFW_KEY_DELETE:
        pkey = Key::DELETE;
        break;
    case GLFW_KEY_HOME:
        pkey = Key::HOME;
        break;
    case GLFW_KEY_END:
        pkey = Key::END;
        break;
    case GLFW_KEY_PAGE_UP:
        pkey = Key::PAGE_UP;
        break;
    case GLFW_KEY_PAGE_DOWN:
        pkey = Key::PAGE_DOWN;
        break;
    case GLFW_KEY_LEFT:
        pkey = Key::LEFT;
        break;
    case GLFW_KEY_RIGHT:
        pkey = Key::RIGHT;
        break;
    case GLFW_KEY_UP:
        pkey = Key::UP;
        break;
    case GLFW_KEY_DOWN:
        pkey = Key::DOWN;
        break;

    // Lock and system keys
    case GLFW_KEY_PRINT_SCREEN:
        pkey = Key::PRINT_SCREEN;
        break;
    case GLFW_KEY_PAUSE:
        pkey = Key::PAUSE;
        break;

    // Main keyboard area symbol keys
    case GLFW_KEY_GRAVE_ACCENT:
        pkey = Key::GRAVE_ACCENT;
        break;
    case GLFW_KEY_MINUS:
        pkey = Key::MINUS;
        break;
    case GLFW_KEY_EQUAL:
        pkey = Key::EQUALS;
        break;
    case GLFW_KEY_LEFT_BRACKET:
        pkey = Key::LEFT_BRACKET;
        break;
    case GLFW_KEY_RIGHT_BRACKET:
        pkey = Key::RIGHT_BRACKET;
        break;
    case GLFW_KEY_BACKSLASH:
        pkey = Key::BACKSLASH;
        break;
    case GLFW_KEY_SEMICOLON:
        pkey = Key::SEMICOLON;
        break;
    case GLFW_KEY_APOSTROPHE:
        pkey = Key::APOSTROPHE;
        break;
    case GLFW_KEY_COMMA:
        pkey = Key::COMMA;
        break;
    case GLFW_KEY_PERIOD:
        pkey = Key::PERIOD;
        break;
    case GLFW_KEY_SLASH:
        pkey = Key::SLASH;
        break;

    // Numpad area
    case GLFW_KEY_KP_0:
        pkey = Key::NUMPAD_0;
        break;
    case GLFW_KEY_KP_1:
        pkey = Key::NUMPAD_1;
        break;
    case GLFW_KEY_KP_2:
        pkey = Key::NUMPAD_2;
        break;
    case GLFW_KEY_KP_3:
        pkey = Key::NUMPAD_3;
        break;
    case GLFW_KEY_KP_4:
        pkey = Key::NUMPAD_4;
        break;
    case GLFW_KEY_KP_5:
        pkey = Key::NUMPAD_5;
        break;
    case GLFW_KEY_KP_6:
        pkey = Key::NUMPAD_6;
        break;
    case GLFW_KEY_KP_7:
        pkey = Key::NUMPAD_7;
        break;
    case GLFW_KEY_KP_8:
        pkey = Key::NUMPAD_8;
        break;
    case GLFW_KEY_KP_9:
        pkey = Key::NUMPAD_9;
        break;
    case GLFW_KEY_KP_ADD:
        pkey = Key::NUMPAD_ADD;
        break;
    case GLFW_KEY_KP_SUBTRACT:
        pkey = Key::NUMPAD_SUBTRACT;
        break;
    case GLFW_KEY_KP_MULTIPLY:
        pkey = Key::NUMPAD_MULTIPLY;
        break;
    case GLFW_KEY_KP_DIVIDE:
        pkey = Key::NUMPAD_DIVIDE;
        break;
    case GLFW_KEY_KP_DECIMAL:
        pkey = Key::NUMPAD_DECIMAL;
        break;
    case GLFW_KEY_KP_ENTER:
        pkey = Key::NUMPAD_ENTER;
        break;

    default:
        Logger::error("Unknown Key {}", key);
        return;
    }

    if (action == GLFW_PRESS) {
        act = KeyAction::PRESS;
    } else if (action == GLFW_RELEASE) {
        act = KeyAction::RELEASE;
    } else {
        act = KeyAction::REPEAT;
    }

    app->dispatch_event(KeyEvent{pkey, act});
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action,
                                int mods) {
    ImGuiIO& io = ImGui::GetIO();
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");
    if (io.WantCaptureMouse && app->m_window.is_mouse_enable()) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        return;
    }
    MouseKey key;
    KeyAction act;
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        key = MouseKey::LEFT_BUTTON;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        key = MouseKey::RIGHT_BUTTON;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        key = MouseKey::MIDDLE_BUTTON;
        break;
    case GLFW_MOUSE_BUTTON_4:
        key = MouseKey::BACK_BUTTON;
        break;
    case GLFW_MOUSE_BUTTON_5:
        key = MouseKey::FORWARD_BUTTON;
        break;
    case GLFW_MOUSE_BUTTON_6:
        key = MouseKey::EXTRA_BUTTON_1;
        break;
    case GLFW_MOUSE_BUTTON_7:
        key = MouseKey::EXTRA_BUTTON_2;
        break;
    case GLFW_MOUSE_BUTTON_8:
        key = MouseKey::EXTRA_BUTTON_3;
        break;
    default:
        Logger::error("Unknown Mouse Button {}", button);
        return;
    }
    if (action == GLFW_PRESS) {
        act = KeyAction::PRESS;
    } else if (action == GLFW_RELEASE) {
        act = KeyAction::RELEASE;
    } else {
        act = KeyAction::REPEAT;
    }

    app->dispatch_event(MouseButtonEvent{key, act});
}

void App::window_focus_callback(GLFWwindow* window, int focused) {
    ImGuiIO& io = ImGui::GetIO();
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");
    if (io.WantCaptureMouse && app->m_window.is_mouse_enable()) {
        ImGui_ImplGlfw_WindowFocusCallback(window, focused);
        return;
    }
    if (focused) {
        auto camera = app->m_window.camera();
        if (camera) {
            camera->reset_camera();
        }
    }
}

void App::window_reshape_callback(GLFWwindow* window, int width, int height) {

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");

    app->dispatch_event(WindowResizeEvent{width, height});

    Logger::info("Window Reshape W: {} H: {}", width, height);
}
void App::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");
    app->dispatch_event(FrameBufferResizeEvent{width, height});

    Logger::info("Frame Buffer Reshape W: {} H: {}", width, height);
}
void App::mouse_scroll_callback(GLFWwindow* window, double xoffset,
                                double yoffset) {
    ImGuiIO& io = ImGui::GetIO();

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");
    if (io.WantCaptureMouse && app->m_window.is_mouse_enable()) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        return;
    }
    app->dispatch_event(MouseWheelEvent(static_cast<float>(yoffset)));
}

void App::cursor_enter_callback(GLFWwindow* window, int entered) {
    ImGuiIO& io = ImGui::GetIO();
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");
    if (io.WantCaptureMouse && app->m_window.is_mouse_enable()) {
        ImGui_ImplGlfw_CursorEnterCallback(window, entered);
        return;
    }
}

void App::char_callback(GLFWwindow* window, unsigned int c) {
    ImGuiIO& io = ImGui::GetIO();
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    ASSERT_MSG(app, "nullptr");
    if (io.WantCaptureKeyboard && app->m_window.is_mouse_enable()) {
        ImGui_ImplGlfw_CharCallback(window, c);
    }
}

void App::render() {

    if (glfwGetWindowAttrib(m_window.get_glfw_window(), GLFW_ICONIFIED) != 0) {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }
    m_renderer.begin_frame();
    m_scene_manager.render(m_renderer);
    m_renderer.end_frame();
    glfwSwapBuffers(m_window.get_glfw_window());
}

void App::run() {

    last_time = glfwGetTime();
    while (!glfwWindowShouldClose(m_window.get_glfw_window())) {
        // if (m_client_world.is_receive_exit()) {
        //     break;
        // }
        update();
        render();
    }

    if (!m_argument.is_client) {
        m_server.server_world().stop();
    }
}
// static Gait player_gait = Gait::WALK;
void App::update() {
    glfwPollEvents();
    {
        int w, h;
        glfwGetFramebufferSize(m_window.get_glfw_window(), &w, &h);

        if (w != m_renderer.frame_width() || h != m_renderer.frame_height()) {
            dispatch_event(FrameBufferResizeEvent{w, h});
        }
    }
    {
        int w, h;
        glfwGetWindowSize(m_window.get_glfw_window(), &w, &h);

        if (w != m_renderer.window_width() || h != m_renderer.window_height()) {
            dispatch_event(WindowResizeEvent{w, h});
        }
    }

    current_time = glfwGetTime();
    dt = current_time - last_time;
    last_time = current_time;
    fps_time_count += dt;
    frame_count++;
    if (fps_time_count >= 1.0f) {
        fps = static_cast<int>(frame_count / fps_time_count);
        std::string title = "Cubed FPS: " + std::to_string(fps);
        glfwSetWindowTitle(m_window.get_glfw_window(), title.c_str());
        frame_count = 0;
        fps_time_count = 0.0f;
        DebugCollector::get().report(
            "fps", std::string{"FPS: " + std::to_string(fps)});
        DebugCollector::get().report(
            "rss",
            std::format("RSS: {}mb", Tools::get_current_rss() / (1024 * 1024)));
    }
    m_texture_manager.update();

    m_audio.update();
    DebugCollector::get().get_widget().update(dt);
    m_renderer.update(dt);

    m_scene_manager.update(dt);
}

void App::dispatch_event(const Event& e) {
    if (m_window.handle_event(e)) {
        return;
    }

    if (m_texture_manager.handle_event(e)) {
        return;
    }

    if (m_renderer.handle_event(e)) {
        return;
    }

    if (m_scene_manager.handle_event(e)) {
        return;
    }
}

int App::start_cubed_application(int argc, char** argv) {

    App app;

    try {
        app.init(argc, argv);
        Logger::info("Game Init Finish Start Run...");
        app.run();

        return 0;
    } catch (std::exception& e) {
        Logger::error("{}", e.what());

    } catch (...) {
        Logger::error("Unkown error");
    }
    return 1;
}

float App::delta_time() { return dt; }

float App::get_fps() { return fps; }

Renderer& App::renderer() { return m_renderer; }
TextureManager& App::texture_manager() { return m_texture_manager; }
Window& App::window() { return m_window; }
ServerWorld& App::server_world() { return m_server.server_world(); }
Config& App::config() { return m_game_config; }
const Argument& App::argument() const { return m_argument; }
AudioEngine& App::audio() { return m_audio; }
} // namespace Cubed