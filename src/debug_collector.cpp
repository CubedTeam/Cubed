#include "Cubed/debug_collector.hpp"

#include "Cubed/tools/system_info.hpp"
#include "Cubed/tools/system_window_manager.hpp"
#include "Cubed/ui/rect.hpp"
#include "version.hpp"

#include <SDL3/SDL_video.h>
#include <memory>

namespace Cubed {

DebugCollector::DebugCollector() : m_widget(nullptr) {}

DebugCollector& DebugCollector::get() { return *get_ptr(); }
std::unique_ptr<DebugCollector>& DebugCollector::get_ptr() {
    static std::unique_ptr<DebugCollector> instance =
        std::make_unique<DebugCollector>();
    return instance;
}
void DebugCollector::destory() { get_ptr().reset(); }
void DebugCollector::init(int, int) {
    constexpr float SCALE = 0.7f;
    constexpr Color COLOR = Color::GRAY;
    constexpr float ALPHA = 0.6f;

    m_widget.set_spacing(15);
    m_widget.set_anchor(Anchor::TOP_LEFT);
    m_widget.set_offset({0, 5});
    m_widget.set_child_anchor(ColumnLayoutAnchor::LEFT);

    auto add_label = [&](std::string_view text, std::string_view key = "") {
        auto& label = m_widget.add_child<Label>();
        auto rect = std::make_unique<Rect>(&label);
        rect->set_color(COLOR).set_alpha(ALPHA).set_fill_parent(true);
        label.set_background(std::move(rect));
        label.set_color(Color::WHITE).set_text(text).set_scale(SCALE);
        if (!key.empty()) {
            auto [_, insert] =
                m_component.try_emplace(std::string(key), &label);
            if (!insert) {
                Logger::error("DebugCollector Key {} already esist", key);
            }
        }
    };

    // version_text

    std::string version{"Version: " CUBED_VERSION};

#ifdef DEBUG_MODE
    version.append("-debug");
#else
    version.append("-release");
#endif

    add_label(version);

    add_label(Tools::get_compiler_info());

    // fps

    add_label("FPS: 0", "fps");

    // player_pos
    add_label("x: 0.00 y: 0.00 z: 0.00", "player_pos");

    // rendered_chunk
    add_label("Rendered Chunk: 0", "rendered_chunk");

    // rss
    add_label("RSS: 0mb", "rss");

    // os
    std::string os;
    std::string os_text;
    if (Tools::get_os_version(os)) {
        os_text = "OS: " + os;
        Logger::info("OS System: {}", os);
    } else {
        os_text = "OS: Unknown";
    }
    add_label(os_text);
    {
        std::string wm{"WM: "};
        wm.append(Tools::detect_wm());
        add_label(wm);
    }
    // cpu
    add_label("CPU: " + Tools::get_cpu_info());

    // gpu
    add_label(std::string{"GPU: "} +
              reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    // opengl_version
    add_label("OpenGL: " + std::to_string(GLVersion.major) + "." +
              std::to_string(GLVersion.minor));

    add_label(std::format("VideoDiver: {}", SDL_GetCurrentVideoDriver()));

    // speed
    add_label("Speed: 0 m/s", "speed");

    add_label("Window W: {} H: {}", "window_size");

    add_label("FrameBuffer W: {} H: {}", "frame_buffer");
}

Widget& DebugCollector::get_widget() { return m_widget; }

bool DebugCollector::handle_event(const Event& e) {
    return std::visit(
        Overloaded{[this](const MouseMoveEvent& e) {
                       if (m_widget.handle_mouse_move_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const MouseButtonEvent& e) {
                       if (m_widget.handle_mouse_button_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const MouseWheelEvent& e) {
                       if (m_widget.handle_mouse_wheel_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const KeyEvent& e) {
                       if (m_widget.handle_key_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [](const TextInputEvent&) { return false; },
                   [this](const WindowResizeEvent& e) {
                       m_widget.handle_window_resize_event(e);
                       return false;
                   },
                   [](const FrameBufferResizeEvent&) { return false; }

        },
        e);
}

void DebugCollector::report(const std::string& name, std::string_view content) {
    auto t = m_component.find(name);
    if (t == m_component.end()) {
        Logger::error("DebugCollector: Can't Find {}", name);
        return;
    }
    if (!t->second) {
        return;
    }
    t->second->set_text(content);
}

} // namespace Cubed