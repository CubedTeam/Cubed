#include "Cubed/scene/settings_scene.hpp"

#include "Cubed/app.hpp"
#include "Cubed/config.hpp"
#include "Cubed/scene/scene_manager.hpp"
namespace Cubed {
SettingsScene::SettingsScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_settings_ui(*this) {}
SettingsScene::~SettingsScene() {}

void SettingsScene::update(float dt) { m_settings_ui.update(dt); }
void SettingsScene::render(Renderer& renderer) {
    m_settings_ui.render(renderer);
}
bool SettingsScene::handle_event(const Event& e) {
    if (m_settings_ui.handle_event(e)) {
        return true;
    }
    return false;
}
void SettingsScene::on_enter() {
    m_settings_ui.init();
    auto& config = m_scene_manager.app().config();
    m_slider_variable.fov = config.get("player.fov", m_slider_variable.fov);
    m_slider_variable.mouse_sensitivity = config.get(
        "player.mouse_sensitivity", m_slider_variable.mouse_sensitivity);
    m_slider_variable.music =
        config.get("volume.music", m_slider_variable.music);
    m_slider_variable.sfx = config.get("volume.SFX", m_slider_variable.sfx);
    m_slider_variable.rendering_distance = config.get(
        "world.rendering_distance", m_slider_variable.rendering_distance);
}
void SettingsScene::on_leave() {
    save_and_apply();

    if (m_need_texture_reload) {
        m_scene_manager.app().texture_manager().need_reload();
    }
}

void SettingsScene::on_re_enter() {
    auto width = m_scene_manager.app().renderer().window_width();
    auto height = m_scene_manager.app().renderer().window_height();

    handle_event(
        WindowResizeEvent{static_cast<int>(width), static_cast<int>(height)});
}

void SettingsScene::save_and_apply() {
    auto& config = m_scene_manager.app().config();
    config.set("player.fov", m_slider_variable.fov);
    config.set("player.mouse_sensitivity", m_slider_variable.mouse_sensitivity);
    config.set("world.rendering_distance",
               m_slider_variable.rendering_distance);
    config.set("volume.SFX", m_slider_variable.sfx);
    config.set("volume.music", m_slider_variable.music);
    config.save_to_file();
    auto& app = m_scene_manager.app();
    app.audio().reload_config();
    app.renderer().reload_config();
}

SceneManager& SettingsScene::scene_manager() { return m_scene_manager; }
SliderVariable& SettingsScene::slider_variable() { return m_slider_variable; }
void SettingsScene::set_texture_reload(bool reload) {
    m_need_texture_reload = reload;
}
} // namespace Cubed