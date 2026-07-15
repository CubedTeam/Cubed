#pragma once
#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/settings_ui.hpp"
namespace Cubed {

struct SliderVariable {
    float fov = 70.0f;
    float mouse_sensitivity = 0.15f;
    float sfx = 1.0f;
    float music = 0.5f;
    int rendering_distance = 24;
};

class SceneManager;
class SettingsScene : public Scene {
public:
    SettingsScene(const SettingsScene&) = delete;
    SettingsScene(SettingsScene&&) = delete;
    SettingsScene& operator=(const SettingsScene&) = delete;
    SettingsScene& operator=(SettingsScene&&) = delete;
    SettingsScene(SceneManager& scene_manager);
    ~SettingsScene();

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;
    void on_re_enter() override;
    SceneManager& scene_manager();
    SliderVariable& slider_variable();
    void set_texture_reload(bool reload);

private:
    void save_and_apply();
    SceneManager& m_scene_manager;
    SliderVariable m_slider_variable;
    SettingsUI m_settings_ui;
    bool m_need_texture_reload = false;
};
} // namespace Cubed