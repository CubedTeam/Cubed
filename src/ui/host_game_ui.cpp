#include "Cubed/ui/host_game_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/scene/host_game_scene.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/text_field.hpp"
namespace Cubed {
HostGameUI::HostGameUI(HostGameScene& scene) : m_scene(scene) {}
void HostGameUI::init() {
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
    layout.set_anchor(Anchor::CENTER);
    layout.set_offset({0, 20});
    layout.set_spacing(20.0f);
    auto& param = m_scene.scene_manager().world_scene_param();
    param.host_game = true;
    {
        auto& label = layout.add_child<Label>();
        label.set_text("Create A New World");
        label.set_scale(0.7f);
    }
    {
        auto& text_seed = layout.add_child<TextField>();
        text_seed.set_show_text("WorldSeed");
        text_seed.set_app(&m_scene.scene_manager().app());
        text_seed.set_default_image(texture_manager);
        text_seed.set_on_finish([this, &text_seed]() {
            unsigned seed = 0;
            auto& text = text_seed.input_text();
            auto r =
                std::from_chars(text.data(), text.data() + text.size(), seed);

            if (r.ec != std::errc{} || r.ptr != text.data() + text.size()) {
                Logger::error("Invalid seed: {}", text);
                return;
            }
            m_scene.scene_manager().world_scene_param().seed = seed;
        });
    }
    {
        auto& text_port = layout.add_child<TextField>();
        text_port.set_default_image(texture_manager);
        text_port.set_show_text("Port");
        text_port.set_app(&m_scene.scene_manager().app());
        text_port.set_on_finish([this, &text_port]() {
            int port = 25530;
            auto& text = text_port.input_text();
            auto r =
                std::from_chars(text.data(), text.data() + text.size(), port);
            if (r.ec != std::errc{} || r.ptr != text.data() + text.size()) {
                Logger::error("Invalid port: {}", text);
                return;
            }
            if (port > 65535 || port < 0) {

                Logger::error("Port {} out of range", port);
                return;
            }
            m_scene.scene_manager().world_scene_param().port = port;
        });
    }
    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text("Start Game");
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_change(SceneType::WORLD);
        });
    }
    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text("Return");
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_pop();
        });
    }
    m_root_widget = std::move(bi);
}
void HostGameUI::on_re_enter() {}
} // namespace Cubed