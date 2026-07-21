#include "Cubed/app.hpp"

#include "Cubed/config.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/tools/arg_parser.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/font.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/system_info.hpp"
#include "Cubed/tools/system_locate.hpp"
#include "version.hpp"

#include <exception>
#include <imgui_impl_sdl3.h>

namespace Cubed {

App::App()

    : m_game_config(ASSETS_PATH "config.toml"), m_window(m_game_config),
      m_texture_manager(m_game_config), m_audio(m_game_config),
      m_renderer(m_texture_manager, m_game_config), m_scene_manager(*this) {}

App::~App() {
    stop_text_input();
    if (m_opengl_init) {
        Font::destroy();
        DebugCollector::destory();
    }
}

void App::init(int argc, char** argv) {
    bool debug = false;
    Logger::set_console_write(true);
#ifdef DEBUG_MODE
    Logger::set_level(Logger::Level::DEBUG);
    Logger::set_file_write(false);
    debug = true;

#else
    Logger::set_level(Logger::Level::INFO);
    debug = false;
#endif
    handle_argument(argc, argv);
    if (m_argument.log_level) {
        Logger::set_level(Logger::get_level(*m_argument.log_level));
    }
    if (m_argument.logs_path) {
        Logger::set_logs_path(*m_argument.logs_path);
        Logger::set_file_write(true);
    }
    if (!debug) {
        Logger::set_file_write(true);
        Logger::set_console_write(false);
    }
    if (m_argument.enable_consolelog) {
        Logger::set_console_write(true);
    }
    if (m_argument.enable_filelog) {
        Logger::set_file_write(true);
    }
    m_game_config.load_config();
    if (m_argument.language) {
        Localization::instance().load_language(*m_argument.language);
        m_game_config.set("language", *m_argument.language);
    } else {
        auto locate = get_system_locale();
        std::string default_value = "en_US";
        if (locate.country == "CN") {
            default_value = "zh_CN";
        }
        Localization::instance().load_language(
            m_game_config.get("language", default_value));
    }

    m_window.init(m_argument);
    m_window.imgui_init();
    m_opengl_init = true;
    Logger::info("Window Init Success");

    m_audio.init();
    BlockManager::init();
    m_renderer.init();
    Logger::info("Renderer Init Success");
    // MapTable::init_map();
    m_texture_manager.init_texture();
    Logger::info("Texture Load Success");

    m_scene_manager.request_push(SceneType::MAIN_MENU);
    last_tick = SDL_GetTicks();
    current_tick = SDL_GetTicks();
    {
        int w, h;
        SDL_GetWindowSize(m_window.get_window(), &w, &h);
        handle_window_resize(w, h);
    }

    {
        int w, h;
        SDL_GetWindowSizeInPixels(m_window.get_window(), &w, &h);
        handle_framebuffer_resize(w, h);
    }
}

void App::handle_argument(int argc, char** argv) {

    static const std::unordered_map<std::string_view,
                                    std::function<void(ArgParser&)>>
        HANDLERS{

            {"-p",
             [&](ArgParser& p) {
                 auto arg = p.require_next("-p");
                 int port;
                 auto r =
                     std::from_chars(arg.data(), arg.data() + arg.size(), port);

                 if (r.ec != std::errc{} || r.ptr != arg.data() + arg.size()) {
                     throw std::runtime_error(
                         std::format("Invalid port: {}", arg));
                 }
                 if (port > 65535) {
                     throw std::runtime_error(
                         std::format("Port {} out of range", port));
                 }
                 m_argument.port = port;
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
            {"--language",
             [&](ArgParser& p) {
                 auto arg = p.require_next("--language");
                 m_argument.language = arg;
             }},
            {"--video-driver",
             [&](ArgParser& p) {
                 auto arg = p.require_next("--video-driver");
                 m_argument.video_driver = arg;
             }},
            {"--enable-exclusive",
             [&](ArgParser&) { m_argument.enable_exclusive = true; }},
            {"--logs-path",
             [&](ArgParser& p) {
                 m_argument.logs_path = p.require_next("--logs-path");
             }},
            {"--log-level",
             [&](ArgParser& p) {
                 auto arg = p.require_next("--log-level");
                 int level;
                 auto r = std::from_chars(arg.data(), arg.data() + arg.size(),
                                          level);

                 if (r.ec != std::errc{} || r.ptr != arg.data() + arg.size()) {
                     throw std::runtime_error(
                         std::format("Invalid log Level: {}", arg));
                 }
                 if (level > 3) {
                     throw std::runtime_error(
                         std::format("Level {} out of range", level));
                 }
                 m_argument.log_level = level;
             }},
            {"--enable-filelog",
             [&](ArgParser&) { m_argument.enable_filelog = true; }},
            {"--enale-consolelog",
             [&](ArgParser&) { m_argument.enable_consolelog = true; }}

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
void App::handle_mouse_move(float xpos, float ypos, float xrel, float yrel) {

    m_scene_manager.handle_event(MouseMoveEvent{xpos, ypos, xrel, yrel});
}

void App::handle_sdl_key(SDL_Event& e) {
    if (e.type != SDL_EVENT_KEY_DOWN && e.type != SDL_EVENT_KEY_UP) {
        ASSERT_MSG(false, "Hanle SDL Event Error Unknown Key Event");
        return;
    }

    Key key;

    switch (e.key.key) {
    // Letter keys
    case SDLK_A:
        key = Key::A;
        break;
    case SDLK_B:
        key = Key::B;
        break;
    case SDLK_C:
        key = Key::C;
        break;
    case SDLK_D:
        key = Key::D;
        break;
    case SDLK_E:
        key = Key::E;
        break;
    case SDLK_F:
        key = Key::F;
        break;
    case SDLK_G:
        key = Key::G;
        break;
    case SDLK_H:
        key = Key::H;
        break;
    case SDLK_I:
        key = Key::I;
        break;
    case SDLK_J:
        key = Key::J;
        break;
    case SDLK_K:
        key = Key::K;
        break;
    case SDLK_L:
        key = Key::L;
        break;
    case SDLK_M:
        key = Key::M;
        break;
    case SDLK_N:
        key = Key::N;
        break;
    case SDLK_O:
        key = Key::O;
        break;
    case SDLK_P:
        key = Key::P;
        break;
    case SDLK_Q:
        key = Key::Q;
        break;
    case SDLK_R:
        key = Key::R;
        break;
    case SDLK_S:
        key = Key::S;
        break;
    case SDLK_T:
        key = Key::T;
        break;
    case SDLK_U:
        key = Key::U;
        break;
    case SDLK_V:
        key = Key::V;
        break;
    case SDLK_W:
        key = Key::W;
        break;
    case SDLK_X:
        key = Key::X;
        break;
    case SDLK_Y:
        key = Key::Y;
        break;
    case SDLK_Z:
        key = Key::Z;
        break;

    // Digit keys
    case SDLK_0:
        key = Key::DIGIT_0;
        break;
    case SDLK_1:
        key = Key::DIGIT_1;
        break;
    case SDLK_2:
        key = Key::DIGIT_2;
        break;
    case SDLK_3:
        key = Key::DIGIT_3;
        break;
    case SDLK_4:
        key = Key::DIGIT_4;
        break;
    case SDLK_5:
        key = Key::DIGIT_5;
        break;
    case SDLK_6:
        key = Key::DIGIT_6;
        break;
    case SDLK_7:
        key = Key::DIGIT_7;
        break;
    case SDLK_8:
        key = Key::DIGIT_8;
        break;
    case SDLK_9:
        key = Key::DIGIT_9;
        break;

    // Function keys
    case SDLK_F1:
        key = Key::F1;
        break;
    case SDLK_F2:
        key = Key::F2;
        break;
    case SDLK_F3:
        key = Key::F3;
        break;
    case SDLK_F4:
        key = Key::F4;
        break;
    case SDLK_F5:
        key = Key::F5;
        break;
    case SDLK_F6:
        key = Key::F6;
        break;
    case SDLK_F7:
        key = Key::F7;
        break;
    case SDLK_F8:
        key = Key::F8;
        break;
    case SDLK_F9:
        key = Key::F9;
        break;
    case SDLK_F10:
        key = Key::F10;
        break;
    case SDLK_F11:
        key = Key::F11;
        break;
    case SDLK_F12:
        key = Key::F12;
        break;

    // Control keys
    case SDLK_BACKSPACE:
        key = Key::BACKSPACE;
        break;
    case SDLK_TAB:
        key = Key::TAB;
        break;
    case SDLK_RETURN:
        key = Key::ENTER;
        break;
    case SDLK_ESCAPE:
        key = Key::ESCAPE;
        break;
    case SDLK_SPACE:
        key = Key::SPACE;
        break;
    case SDLK_CAPSLOCK:
        key = Key::CAPS_LOCK;
        break;
    case SDLK_NUMLOCKCLEAR:
        key = Key::NUM_LOCK;
        break;
    case SDLK_SCROLLLOCK:
        key = Key::SCROLL_LOCK;
        break;

    // Modifier keys
    case SDLK_LSHIFT:
        key = Key::LEFT_SHIFT;
        break;
    case SDLK_RSHIFT:
        key = Key::RIGHT_SHIFT;
        break;
    case SDLK_LCTRL:
        key = Key::LEFT_CTRL;
        break;
    case SDLK_RCTRL:
        key = Key::RIGHT_CTRL;
        break;
    case SDLK_LALT:
        key = Key::LEFT_ALT;
        break;
    case SDLK_RALT:
        key = Key::RIGHT_ALT;
        break;
    case SDLK_LGUI:
        key = Key::LEFT_SUPER;
        break;
    case SDLK_RGUI:
        key = Key::RIGHT_SUPER;
        break;

    // Navigation keys
    case SDLK_INSERT:
        key = Key::INSERT;
        break;
    case SDLK_DELETE:
        key = Key::DELETE;
        break;
    case SDLK_HOME:
        key = Key::HOME;
        break;
    case SDLK_END:
        key = Key::END;
        break;
    case SDLK_PAGEUP:
        key = Key::PAGE_UP;
        break;
    case SDLK_PAGEDOWN:
        key = Key::PAGE_DOWN;
        break;

    case SDLK_LEFT:
        key = Key::LEFT;
        break;
    case SDLK_RIGHT:
        key = Key::RIGHT;
        break;
    case SDLK_UP:
        key = Key::UP;
        break;
    case SDLK_DOWN:
        key = Key::DOWN;
        break;

    // System keys
    case SDLK_PRINTSCREEN:
        key = Key::PRINT_SCREEN;
        break;
    case SDLK_PAUSE:
        key = Key::PAUSE;
        break;

    // Symbol keys
    case SDLK_GRAVE:
        key = Key::GRAVE_ACCENT;
        break;
    case SDLK_MINUS:
        key = Key::MINUS;
        break;
    case SDLK_EQUALS:
        key = Key::EQUALS;
        break;
    case SDLK_LEFTBRACKET:
        key = Key::LEFT_BRACKET;
        break;
    case SDLK_RIGHTBRACKET:
        key = Key::RIGHT_BRACKET;
        break;
    case SDLK_BACKSLASH:
        key = Key::BACKSLASH;
        break;
    case SDLK_SEMICOLON:
        key = Key::SEMICOLON;
        break;
    case SDLK_APOSTROPHE:
        key = Key::APOSTROPHE;
        break;
    case SDLK_COMMA:
        key = Key::COMMA;
        break;
    case SDLK_PERIOD:
        key = Key::PERIOD;
        break;
    case SDLK_SLASH:
        key = Key::SLASH;
        break;

    // Numpad
    case SDLK_KP_0:
        key = Key::NUMPAD_0;
        break;
    case SDLK_KP_1:
        key = Key::NUMPAD_1;
        break;
    case SDLK_KP_2:
        key = Key::NUMPAD_2;
        break;
    case SDLK_KP_3:
        key = Key::NUMPAD_3;
        break;
    case SDLK_KP_4:
        key = Key::NUMPAD_4;
        break;
    case SDLK_KP_5:
        key = Key::NUMPAD_5;
        break;
    case SDLK_KP_6:
        key = Key::NUMPAD_6;
        break;
    case SDLK_KP_7:
        key = Key::NUMPAD_7;
        break;
    case SDLK_KP_8:
        key = Key::NUMPAD_8;
        break;
    case SDLK_KP_9:
        key = Key::NUMPAD_9;
        break;

    case SDLK_KP_PLUS:
        key = Key::NUMPAD_ADD;
        break;
    case SDLK_KP_MINUS:
        key = Key::NUMPAD_SUBTRACT;
        break;
    case SDLK_KP_MULTIPLY:
        key = Key::NUMPAD_MULTIPLY;
        break;
    case SDLK_KP_DIVIDE:
        key = Key::NUMPAD_DIVIDE;
        break;
    case SDLK_KP_PERIOD:
        key = Key::NUMPAD_DECIMAL;
        break;
    case SDLK_KP_ENTER:
        key = Key::NUMPAD_ENTER;
        break;

    default:
        Logger::error("Unknown Key {}", e.key.key);
        return;
    }

    KeyAction act;

    if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.repeat) {
            act = KeyAction::REPEAT;
        } else {
            act = KeyAction::PRESS;
        }
    } else if (e.type == SDL_EVENT_KEY_UP) {
        act = KeyAction::RELEASE;
    } else {
        ASSERT_MSG(false, "Unknown key event");
        return;
    }

    dispatch_event(KeyEvent{key, act});
}

void App::handle_sdl_mouse_button(SDL_Event& e) {
    if (e.type != SDL_EVENT_MOUSE_BUTTON_DOWN &&
        e.type != SDL_EVENT_MOUSE_BUTTON_UP) {
        ASSERT_MSG(false, "Hanle SDL Event Error: Unknown Mouse Event");
        return;
    }
    MouseKey key;
    KeyAction act;

    switch (e.button.button) {
    case SDL_BUTTON_LEFT:
        key = MouseKey::LEFT_BUTTON;
        break;

    case SDL_BUTTON_RIGHT:
        key = MouseKey::RIGHT_BUTTON;
        break;

    case SDL_BUTTON_MIDDLE:
        key = MouseKey::MIDDLE_BUTTON;
        break;

    case SDL_BUTTON_X1:
        key = MouseKey::BACK_BUTTON;
        break;

    case SDL_BUTTON_X2:
        key = MouseKey::FORWARD_BUTTON;
        break;

    default:
        Logger::error("Unknown Mouse Button {}", e.button.button);
        return;
    }

    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        act = KeyAction::PRESS;
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        act = KeyAction::RELEASE;
    } else {
        ASSERT_MSG(false, "Unknown SDL Mouse Event");
        return;
    }
    dispatch_event(MouseButtonEvent{key, act});
}

void App::handle_window_focus(bool focused) {

    if (focused) {
    }
}

void App::handle_window_resize(int width, int height) {
    Widget::set_window_size(width, height);
    dispatch_event(WindowResizeEvent{width, height});
    d_rep("window_size", "Window W: {} H: {}", width, height);
    Logger::info("Window Reshape W: {} H: {}", width, height);
}
void App::handle_framebuffer_resize(int width, int height) {
    dispatch_event(FrameBufferResizeEvent{width, height});
    d_rep("frame_buffer", "FrameBuffer W: {} H: {}", width, height);
    Logger::info("Frame Buffer Reshape W: {} H: {}", width, height);
}
void App::handle_mouse_scroll(float, float yoffset) {
    dispatch_event(MouseWheelEvent(yoffset));
}

void App::handle_text_input(const char* text) {
    dispatch_event(TextInputEvent{std::string(text)});
}

void App::render() {

    if (SDL_GetWindowFlags(m_window.get_window()) & SDL_WINDOW_MINIMIZED) {
        SDL_Delay(10); // Sleep for 10 milliseconds
        return;        // Skip rendering this frame
    }
    m_renderer.begin_frame();
    m_scene_manager.render(m_renderer);
    m_renderer.end_frame();
    SDL_GL_SwapWindow(m_window.get_window());
}

void App::run() {

    last_tick = SDL_GetTicks();
    while (m_running) {
        update();
        render();
    }
}
// static Gait player_gait = Gait::WALK;
void App::update() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        handle_sdl_event(e);
    }

    current_tick = SDL_GetTicks();
    dt = (current_tick - last_tick) / 1000.0f;
    last_tick = current_tick;
    fps_time_count += dt;
    frame_count++;
    if (fps_time_count >= 1.0f) {
        fps = static_cast<int>(frame_count / fps_time_count);
        std::string title = "Cubed FPS: " + std::to_string(fps);

        SDL_SetWindowTitle(m_window.get_window(), title.c_str());

        frame_count = 0;
        fps_time_count = 0.0f;
        d_rep("fps", "FPS: {}", fps);
        d_rep("rss", "RSS: {}mb", Tools::get_current_rss() / (1024 * 1024));
    }
    m_texture_manager.update();

    m_audio.update();
    DebugCollector::get().get_widget().update(dt);
    m_renderer.update(dt);

    m_scene_manager.update(dt);
}

void App::handle_sdl_event(SDL_Event& e) {
    bool imgui_enable = m_window.is_enable_imgui();
    if (imgui_enable) {
        ImGui_ImplSDL3_ProcessEvent(&e);
    }

    switch (e.type) {
    case SDL_EVENT_QUIT:
        m_running = false;
        break;
    case SDL_EVENT_KEY_DOWN:
        if (imgui_enable &&
            (e.key.key == SDLK_ESCAPE || e.key.key == SDLK_LALT)) {
            m_window.disable_mouse();
            m_window.set_imgui_enabled(false);
        }
        if (!imgui_enable) {
            handle_sdl_key(e);
        }
        break;
    case SDL_EVENT_KEY_UP:
        if (!imgui_enable) {
            handle_sdl_key(e);
        }
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (!imgui_enable) {
            handle_sdl_mouse_button(e);
        }

        break;
    case SDL_EVENT_MOUSE_MOTION:
        if (!imgui_enable) {
            if (std::abs(e.motion.xrel) > 200 ||
                std::abs(e.motion.yrel) > 200) {
                return;
            }
            handle_mouse_move(e.motion.x, e.motion.y, e.motion.xrel,
                              e.motion.yrel);
        }

        break;
    case SDL_EVENT_WINDOW_RESIZED:

        handle_window_resize(e.window.data1, e.window.data2);

        break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:

        handle_framebuffer_resize(e.window.data1, e.window.data2);

        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (!imgui_enable) {
            float scroll_x = e.wheel.x;
            float scroll_y = e.wheel.y;
            if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                scroll_y = -scroll_y;
            }
            handle_mouse_scroll(scroll_x, scroll_y);
        }

        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        handle_window_focus(true);
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        handle_window_focus(false);
        break;
    case SDL_EVENT_TEXT_INPUT:
        if (!imgui_enable) {
            handle_text_input(e.text.text);
        }

        break;
    }
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

    if (DebugCollector::get().handle_event(e)) {
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
        Logger::info("Init Finish Start Run...");
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

Config& App::config() { return m_game_config; }
const Argument& App::argument() const { return m_argument; }
AudioEngine& App::audio() { return m_audio; }
std::string App::get_clipboard_text() {
    char* text = SDL_GetClipboardText();
    if (text) {
        std::string str{text};
        SDL_free(text);
        return str;
    }
    return {};
}

void App::start_text_input() {
    Logger::debug("Start Text Input");
    if (!SDL_StartTextInput(m_window.get_window())) {
        Logger::error("Start Text Input Fail: {}", SDL_GetError());
    }
}
void App::stop_text_input() {
    Logger::debug("Stop Text Input");
    SDL_StopTextInput(m_window.get_window());
}

void App::update_text_input_area(const glm::vec4& textbox,
                                 float cursor_position_x) {

    SDL_Rect area{static_cast<int>(textbox.x), static_cast<int>(textbox.y),
                  static_cast<int>(textbox.z), static_cast<int>(textbox.w)};

    int cursor_x = cursor_position_x - textbox.x;

    SDL_SetTextInputArea(m_window.get_window(), &area, cursor_x);
}

} // namespace Cubed