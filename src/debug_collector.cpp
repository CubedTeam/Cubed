#include "Cubed/debug_collector.hpp"

#include "Cubed/tools/system_info.hpp"
#include "version.hpp"

namespace Cubed {

DebugCollector::DebugCollector() {}

DebugCollector& DebugCollector::get() {
    static DebugCollector instance;
    return instance;
}

void DebugCollector::init_text() {
    // version_text
    auto& version_text = m_widget.add_child<Label>("version");
    std::string version{"Version: " CUBED_VERSION};
#ifdef DEBUG_MODE
    version.append("-debug");
#else
    version.append("-release");
#endif
    version_text.set_color(Color::WHITE)
        .set_text(version)
        .set_scale(0.8f)
        .set_position(0.0f, 100.0f);
    m_component.try_emplace(version_text.id(), &version_text);

    // fps
    auto& fps_text = m_widget.add_child<Label>("fps");
    fps_text.set_text("FPS: 0").set_position(0.0f, 50.0f);
    m_component.try_emplace(fps_text.id(), &fps_text);

    // player_pos
    auto& player_pos_text = m_widget.add_child<Label>("player_pos");
    player_pos_text.set_text("x: 0.00 y: 0.00 z: 0.00")
        .set_scale(0.8f)
        .set_position(0.0f, 150.0f);
    m_component.try_emplace(player_pos_text.id(), &player_pos_text);

    // rendered_chunk
    auto& rendered_chunk_text = m_widget.add_child<Label>("rendered_chunk");
    rendered_chunk_text.set_text("Rendered Chunk: 0")
        .set_scale(0.8f)
        .set_position(0.0f, 200.0f);
    m_component.try_emplace(rendered_chunk_text.id(), &rendered_chunk_text);

    // rss
    auto& rss_text = m_widget.add_child<Label>("rss");
    rss_text.set_text("RSS: 0mb").set_scale(0.8f).set_position(0.0f, 300.0f);
    m_component.try_emplace(rss_text.id(), &rss_text);

    // os
    std::string os;
    auto& os_text = m_widget.add_child<Label>("os");
    os_text.set_scale(0.8f).set_position(0.0f, 250.0f);
    if (Tools::get_os_version(os)) {
        os_text.set_text("OS: " + os);
        Logger::info("System: {}", os);
    } else {
        os_text.set_text("OS: Unknown");
    }
    m_component.try_emplace(os_text.id(), &os_text);

    // cpu
    auto& cpu_text = m_widget.add_child<Label>("cpu");
    cpu_text.set_text("CPU: " + Tools::get_cpu_info())
        .set_scale(0.7f)
        .set_position(0.0f, 350.0f);
    m_component.try_emplace(cpu_text.id(), &cpu_text);

    // gpu
    auto& gpu_text = m_widget.add_child<Label>("gpu");
    gpu_text
        .set_text(std::string{"GPU: "} +
                  reinterpret_cast<const char*>(glGetString(GL_RENDERER)))
        .set_scale(0.7f)
        .set_position(0.0f, 400.0f);
    m_component.try_emplace(gpu_text.id(), &gpu_text);

    // opengl_version
    auto& opengl_version_text = m_widget.add_child<Label>("opengl_version");
    opengl_version_text
        .set_text("OpenGL: " + std::to_string(GLVersion.major) + "." +
                  std::to_string(GLVersion.minor))
        .set_scale(0.7f)
        .set_position(0.0f, 450.0f);
    m_component.try_emplace(opengl_version_text.id(), &opengl_version_text);

    // biome
    auto& biome_text = m_widget.add_child<Label>("biome");
    biome_text.set_text("Biome: ").set_scale(0.8f).set_position(0.0f, 500.0f);
    m_component.try_emplace(biome_text.id(), &biome_text);

    // speed
    auto& speed_text = m_widget.add_child<Label>("speed");
    speed_text.set_text("Speed: 0 m/s")
        .set_scale(0.8f)
        .set_position(0.0f, 550.0f);
    m_component.try_emplace(speed_text.id(), &speed_text);
}

Widget& DebugCollector::get_widget() { return m_widget; }

void DebugCollector::report(const std::string& name, std::string_view content) {
    auto t = m_component.find(name);
    if (t == m_component.end()) {
        return;
    }
    if (!t->second) {
        return;
    }
    t->second->set_text(content);
}

} // namespace Cubed