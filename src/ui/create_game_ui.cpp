#include "Cubed/ui/create_game_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/scene/create_game_scene.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/tools/world_name.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/default_image.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/text_field.hpp"

#include <filesystem>
namespace fs = std::filesystem;
namespace Cubed {
CreateGameUI::CreateGameUI(CreateGameScene& scene) : m_scene(scene) {}
void CreateGameUI::init() {
    auto bi = std::make_unique<Image>(nullptr);
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();

    bi->set_anchor(Anchor::TOP_LEFT);
    bi->set_image("cubed/textures/ui/background.png", texture_manager, false);
    bi->set_fill_parent(true);

    auto& rect = bi->add_child<Rect>();
    rect.set_fill_parent(true);
    rect.set_alpha(0.7f);
    rect.set_color(Color::BLACK);

    auto& layout = rect.add_child<ColumnLayout>();
    layout.set_anchor(Anchor::CENTER);
    layout.set_offset({0, 20});
    layout.set_spacing(20.0f);
    auto& param = m_scene.scene_manager().world_scene_param();
    param.host_game = true;
    param.seed = std::nullopt;
    {
        auto& label = layout.add_child<Label>();
        label.set_text(tr("hostgame.create_a_new_world"));
        label.set_scale(0.7f);
    }

    {
        auto& label = layout.add_child<Label>();
        label.set_text(tr("error.no_error"));
        label.set_color(Color::RED);
        label.set_scale(0.7f);
        label.set_visible(false);
        m_error_label = &label;
    }
    {
        auto& text_field = layout.add_child<TextField>();
        text_field.set_show_text(tr("hostgame.world_name"));
        text_field.set_app(&m_scene.scene_manager().app());
        std::unique_ptr<Image> back = std::make_unique<Image>(&text_field);
        back->set_image(DEFAULT_TEXT_FIELD_IMAGE, texture_manager, false)
            .set_fill_parent(true);
        text_field.set_background(std::move(back));
        m_world_name_field = &text_field;

        text_field.set_on_finish([this, &text_field]() {
            auto& name = text_field.input_text();
            if (name.empty()) {
                m_scene.scene_manager().world_scene_param().world_name =
                    "new_world";
            } else if (Tools::is_valid_world_name(name)) {
                m_scene.scene_manager().world_scene_param().world_name = name;
            }
        });
    }
    {
        auto& text_seed = layout.add_child<TextField>();
        text_seed.set_show_text(tr("hostgame.world_seed"));
        text_seed.set_app(&m_scene.scene_manager().app());
        std::unique_ptr<Image> back = std::make_unique<Image>(&text_seed);
        back->set_image(DEFAULT_TEXT_FIELD_IMAGE, texture_manager, false)
            .set_fill_parent(true);
        text_seed.set_background(std::move(back));
        text_seed.set_on_finish([this, &text_seed]() {
            unsigned seed = 0;
            auto& text = text_seed.input_text();
            auto r =
                std::from_chars(text.data(), text.data() + text.size(), seed);

            if (r.ec != std::errc{} || r.ptr != text.data() + text.size()) {
                std::string error = tr("error.invalid_seed", arg("seed", text));
                set_error(error);
                return;
            }
            clear_error();
            m_scene.scene_manager().world_scene_param().seed = seed;
        });
    }
    {
        auto& text_port = layout.add_child<TextField>();
        std::unique_ptr<Image> back = std::make_unique<Image>(&text_port);
        back->set_image(DEFAULT_TEXT_FIELD_IMAGE, texture_manager, false)
            .set_fill_parent(true);
        text_port.set_background(std::move(back));
        text_port.set_show_text(tr("hostgame.port"));
        text_port.set_app(&m_scene.scene_manager().app());
        text_port.set_on_finish([this, &text_port]() {
            int port = 25530;
            auto& text = text_port.input_text();
            auto r =
                std::from_chars(text.data(), text.data() + text.size(), port);
            if (r.ec != std::errc{} || r.ptr != text.data() + text.size()) {
                std::string error =
                    tr("error.invalid_port", arg("port", std::string(text)));

                set_error(error);
                return;
            }
            if (port > 65535 || port < 0) {
                std::string error = tr("error.port_out_of_range",
                                       arg("port", std::to_string(port)));

                set_error(error);
                return;
            }
            clear_error();
            m_scene.scene_manager().world_scene_param().port = port;
        });
    }
    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text(tr("hostgame.create_world"));
        button.set_clicked([this, &button]() {
            const auto& input = m_world_name_field->input_text();
            if (!input.empty() && !Tools::is_valid_world_name(input)) {
                set_error(tr("hostgame.invalid_world_name"));
                return;
            }
            if (input.empty()) {
                set_error(tr("hostgame.empty_world_name"));
                return;
            }

            auto save = ServerWorld::SAVE_ROOT / input;
            std::error_code ec;
            bool exists = fs::exists(save, ec);
            if (ec) {
                set_error(ec.message());
                return;
            }
            if (exists) {
                set_error(tr("hostgame.world_name_already_exists"));
                return;
            }
            clear_error();
            m_scene.scene_manager().world_scene_param().world_name = input;
            button.set_enable(false);
            m_scene.scene_manager().request_change(SceneType::WORLD);
        });
    }
    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text(tr("button.return"));
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_pop();
        });
    }
    m_root_widget = std::move(bi);
}
void CreateGameUI::on_re_enter() {}

void CreateGameUI::set_error(std::string_view error) {
    if (!m_error_label) {
        return;
    }
    m_error_label->set_text(error);
    m_error_label->set_visible(true);
}
void CreateGameUI::clear_error() { m_error_label->set_visible(false); }
} // namespace Cubed
