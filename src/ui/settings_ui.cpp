#include "Cubed/ui/settings_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/settings_scene.hpp"
#include "Cubed/tools/system_locate.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/combo_button.hpp"
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
        auto& title = layout.add_child<Label>();
        title.set_text(tr("settings.title"));
    }
    {
        auto& fov = layout.add_child<Slider>();
        fov.set_slider(&v.fov, 20.0f, 120.0f);
        set_default_slider_image(fov);
        fov.set_slider_text("settings.fov", "fov");

        auto& sensitivity = layout.add_child<Slider>();
        sensitivity.set_slider(&v.mouse_sensitivity, 0.01f, 1.0f);
        set_default_slider_image(sensitivity);
        sensitivity.set_slider_text("settings.mouse_sensitivity",
                                    "sensitivity");

        auto& distance = layout.add_child<Slider>();
        distance.set_slider(&v.rendering_distance, 2, 64);
        set_default_slider_image(distance);
        distance.set_slider_text("settings.distance", "distance");

        auto& music = layout.add_child<Slider>();
        music.set_slider(&v.music, 0.0f, 1.0f);
        set_default_slider_image(music);
        music.set_slider_text("settings.music_volume", "music");

        auto& sfx = layout.add_child<Slider>();
        sfx.set_slider(&v.sfx, 0.0f, 1.0f);
        set_default_slider_image(sfx);
        sfx.set_slider_text("settings.sfx_volume", "sfx");
    }
    auto& config = m_scene.scene_manager().app().config();
    {

        bool full = config.get("window.fullscreen", false);
        auto& fullscreen = layout.add_child<ComboButton>();
        fullscreen.set_index(!full ? 0 : 1);
        fullscreen.set_combo_text("settings.fullscreen", "state");
        std::vector<std::pair<std::string, std::function<void()>>> comb;
        comb.emplace_back(tr("common.off"), [&config, this]() {
            config.set("window.fullscreen", false);
            m_scene.scene_manager().app().window().reload_config();
        });
        comb.emplace_back(tr("common.on"), [&config, this]() {
            config.set("window.fullscreen", true);
            m_scene.scene_manager().app().window().reload_config();
        });
        fullscreen.set_default_image(texture_manager);
        fullscreen.set_combos(comb);
    }

    {
        bool enable_vsync = config.get("window.V-Sync", true);
        auto& vsync = layout.add_child<ComboButton>();
        vsync.set_default_image(texture_manager);
        vsync.set_combo_text("settings.vsync", "state");
        std::vector<ComboPair> combos;
        combos.emplace_back(tr("common.off"), [this, &config]() {
            config.set("window.V-Sync", false);
            m_scene.scene_manager().app().window().reload_config();
        });
        combos.emplace_back(tr("common.on"), [this, &config]() {
            config.set("window.V-Sync", true);
            m_scene.scene_manager().app().window().reload_config();
        });

        vsync.set_combos(combos);
        vsync.set_index(!enable_vsync ? 0 : 1);
    }
    {
        int max_aniso = texture_manager.max_aniso();
        int config_aniso = config.get("texture.aniso", 1);
        int aniso = (config_aniso > max_aniso) ? max_aniso : config_aniso;
        int max_sum = 1;
        for (int temp = max_aniso; temp > 1; temp /= 2) {
            ++max_sum;
        }

        int cur_index = 0;
        for (int i = 0, temp = 1; i < max_sum; i++) {
            if (temp == aniso) {
                cur_index = i;
            }
            temp *= 2;
        }

        auto& aniso_button = layout.add_child<ComboButton>();
        aniso_button.set_default_image(texture_manager);
        aniso_button.set_combo_text("settings.aniso", "aniso");
        aniso_button.set_index(cur_index);
        std::vector<ComboPair> combos;
        auto get_suffix = [](int i) -> std::pair<std::string, int> {
            if (i == 0) {
                return {tr("common.off"), 1};
            }
            int n = 1;
            for (int j = 0; j < i; j++) {
                n *= 2;
            }

            return {std::to_string(n) + "x", n};
        };
        for (int i = 0; i < max_sum; i++) {
            auto [str, n] = get_suffix(i);
            combos.emplace_back(str, [&config, this, n]() {
                config.set("texture.aniso", n);
                m_scene.set_texture_reload(true);
            });
        }
        aniso_button.set_combos(combos);
    }

    {
        auto& lang_button = layout.add_child<ComboButton>();
        auto& label = layout.add_child<Label>();
        label.set_text(tr("settings.restart_game_info"));
        label.set_scale(0.7f);
        label.set_color(Color::RED);
        label.set_visible(false);
        lang_button.set_default_image(texture_manager);
        lang_button.set_combo_text("settings.language", "lang");
        auto locate = get_system_locale();
        std::string default_value = "en_US";
        if (locate.country == "CN") {
            default_value = "zh_CN";
        }
        auto lang = config.get("language", default_value);
        int index = 0;
        if (lang == "en_US") {
            index = 0;
        } else if (lang == "zh_CN") {
            index = 1;
        }
        lang_button.set_index(index);
        std::vector<ComboPair> combos;

        combos.emplace_back("English", [&label, &config]() {
            config.set("language", std::string("en_US"));
            label.set_visible(true);
        });
        combos.emplace_back("简体中文", [&label, &config]() {
            config.set("language", std::string("zh_CN"));
            label.set_visible(true);
        });
        lang_button.set_combos(combos);
    }

    auto& return_button = layout.add_child<Button>();
    return_button.set_background_image("texture/ui/button001.png",
                                       texture_manager);

    return_button.set_text(tr("button.done"));
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