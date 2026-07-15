#include "Cubed/ui/settings_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/settings_scene.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/slider.hpp"
namespace Cubed {
SettingsUI::SettingsUI(SettingsScene& scene) : m_scene(scene) {}
void SettingsUI::init() {
    auto bi = std::make_unique<Image>(nullptr);
    auto& renderer = m_scene.scene_manager().app().renderer();
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();

    bi->set_window_size(renderer.window_width(), renderer.window_height());
    bi->set_anchor(Anchor::TOP_LEFT);
    bi->set_image("texture/ui/background.png", texture_manager);
    bi->set_fill(true);

    auto& rect = bi->add_child<Rect>();
    rect.set_fill(true);
    rect.set_alpha(0.7f);
    rect.set_color(Color::BLACK);

    auto& layout = rect.add_child<ColumnLayout>();
    layout.set_anchor(Anchor::TOP_CENTER);
    layout.set_offset({0, 20});
    layout.set_spacing(20.0f);
    auto& v = m_scene.slider_variable();
    auto set_default_slider_image = [&](Slider& slider) {
        slider.set_thumb_image("texture/ui/slider_thumb001.png",
                               texture_manager);
        slider.set_track_image("texture/ui/slider_track001.png",
                               texture_manager);
    };

    {
        auto& fov = layout.add_child<Slider>();
        fov.set_slider(&v.fov, 20.0f, 120.0f);
        set_default_slider_image(fov);
        fov.set_text("Fov");

        auto& sensitivity = layout.add_child<Slider>();
        sensitivity.set_slider(&v.mouse_sensitivity, 0.01f, 1.0f);
        set_default_slider_image(sensitivity);
        sensitivity.set_text("Sensitivity");

        auto& distance = layout.add_child<Slider>();
        distance.set_slider(&v.rendering_distance, 2, 64);
        set_default_slider_image(distance);
        distance.set_text("Distance");

        auto& music = layout.add_child<Slider>();
        music.set_slider(&v.music, 0.0f, 1.0f);
        set_default_slider_image(music);
        music.set_text("Music");

        auto& sfx = layout.add_child<Slider>();
        sfx.set_slider(&v.sfx, 0.0f, 1.0f);
        set_default_slider_image(sfx);
        sfx.set_text("SFX");
    }

    auto& return_button = layout.add_child<Button>();
    return_button.set_background_image("texture/ui/button001.png",
                                       texture_manager);

    return_button.set_text("Save and Return");
    return_button.set_clicked(
        [this]() { m_scene.scene_manager().request_pop(); });

    m_root_widget = std::move(bi);
}

bool SettingsUI::handle_key_event(const KeyEvent& e) {
    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
        m_scene.scene_manager().request_pop();
        return true;
    }
    return UIManager::handle_key_event(e);
}

} // namespace Cubed