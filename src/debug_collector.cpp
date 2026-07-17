#include "Cubed/debug_collector.hpp"

#include "Cubed/tools/system_info.hpp"
#include "version.hpp"

#include <SDL3/SDL_video.h>

namespace Cubed {

DebugCollector::DebugCollector() : m_widget(nullptr) {}

DebugCollector& DebugCollector::get() {
    static DebugCollector instance;
    return instance;
}

void DebugCollector::init(int width, int height) {
    constexpr float SCALE = 0.6f;

    m_widget.set_window_size(width, height);
    m_widget.set_spacing(15);
    m_widget.set_anchor(Anchor::TOP_LEFT);
    m_widget.set_offset({0, 5});
    m_widget.set_child_anchor(ColumnLayoutAnchor::LEFT);
    // version_text
    auto& version_text = m_widget.add_child<Label>();
    std::string version{"Version: " CUBED_VERSION};

#ifdef DEBUG_MODE
    version.append("-debug");
#else
    version.append("-release");
#endif

    version_text.set_color(Color::WHITE).set_text(version).set_scale(SCALE);

    {
        auto& compiler = m_widget.add_child<Label>();
        compiler.set_text(Tools::get_compiler_info()).set_scale(SCALE);
    }

    // fps
    auto& fps_text = m_widget.add_child<Label>();
    fps_text.set_text("FPS: 0").set_scale(SCALE);
    m_component.try_emplace("fps", &fps_text);

    // player_pos
    auto& player_pos_text = m_widget.add_child<Label>();
    player_pos_text.set_text("x: 0.00 y: 0.00 z: 0.00").set_scale(SCALE);
    m_component.try_emplace("player_pos", &player_pos_text);

    // rendered_chunk
    auto& rendered_chunk_text = m_widget.add_child<Label>();
    rendered_chunk_text.set_text("Rendered Chunk: 0").set_scale(SCALE);
    m_component.try_emplace("rendered_chunk", &rendered_chunk_text);

    // rss
    auto& rss_text = m_widget.add_child<Label>();
    rss_text.set_text("RSS: 0mb").set_scale(SCALE);
    m_component.try_emplace("rss", &rss_text);

    // os
    std::string os;
    auto& os_text = m_widget.add_child<Label>();
    os_text.set_scale(SCALE);
    if (Tools::get_os_version(os)) {
        os_text.set_text("OS: " + os);
        Logger::info("System: {}", os);
    } else {
        os_text.set_text("OS: Unknown");
    }

    // cpu
    auto& cpu_text = m_widget.add_child<Label>();
    cpu_text.set_text("CPU: " + Tools::get_cpu_info()).set_scale(SCALE);

    // gpu
    auto& gpu_text = m_widget.add_child<Label>();
    gpu_text
        .set_text(std::string{"GPU: "} +
                  reinterpret_cast<const char*>(glGetString(GL_RENDERER)))
        .set_scale(SCALE);

    // opengl_version
    auto& opengl_version_text = m_widget.add_child<Label>();
    opengl_version_text
        .set_text("OpenGL: " + std::to_string(GLVersion.major) + "." +
                  std::to_string(GLVersion.minor))
        .set_scale(SCALE);
    {
        auto& video_driver = m_widget.add_child<Label>();
        video_driver
            .set_text(
                std::format("VideoDiver: {}", SDL_GetCurrentVideoDriver()))
            .set_scale(SCALE);
    }
    // speed
    auto& speed_text = m_widget.add_child<Label>();
    speed_text.set_text("Speed: 0 m/s").set_scale(SCALE);
    m_component.try_emplace("speed", &speed_text);
    {
        auto& window_size = m_widget.add_child<Label>();
        window_size.set_text("Window W: {} H: {}").set_scale(SCALE);
        m_component.try_emplace("window_size", &window_size);
    }
    {
        auto& frame_size = m_widget.add_child<Label>();
        frame_size.set_text("FrameBuffer W: {} H: {}").set_scale(SCALE);
        m_component.try_emplace("frame_buffer", &frame_size);
    }
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