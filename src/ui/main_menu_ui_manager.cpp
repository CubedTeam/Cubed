#include "Cubed/ui/main_menu_ui_manager.hpp"

#include "Cubed/app.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/main_menu_scene.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "version.hpp"

namespace Cubed {

MainMenuUIManager::MainMenuUIManager(MainMenuScene& scene) : m_scene(scene) {}
MainMenuUIManager::~MainMenuUIManager() {}

void MainMenuUIManager::init() {

    auto image = std::make_unique<Image>(nullptr);
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    image->set_fill(true);
    image->set_anchor(Anchor::TOP_LEFT);
    image->set_image("texture/ui/background.png", texture_manager);
    auto& renderer = m_scene.scene_manager().app().renderer();
    image->set_window_size(renderer.window_width(), renderer.window_height());

    auto& layout = image->add_child<ColumnLayout>();
    layout.set_spacing(20);
    layout.set_anchor(Anchor::CENTER);
    layout.set_window_size(renderer.window_width(), renderer.window_height());
    {
        auto& start_game_button = layout.add_child<Button>();

        start_game_button.set_background_image("texture/ui/button001.png",
                                               texture_manager);
        start_game_button.set_text(tr("menu.main.host_game"));
        start_game_button.set_clicked([this, &start_game_button]() {
            start_game_button.set_enable(false);
            m_scene.scene_manager().request_push(SceneType::HOST_GAME);
        });
        m_pending_enable.emplace_back(&start_game_button);
    }
    {
        auto& start_game_button = layout.add_child<Button>();

        start_game_button.set_background_image("texture/ui/button001.png",
                                               texture_manager);
        start_game_button.set_text(tr("menu.main.join_game"));
        start_game_button.set_clicked([this, &start_game_button]() {
            start_game_button.set_enable(false);
            m_scene.scene_manager().request_push(SceneType::JOIN_GAME);
        });
        m_pending_enable.emplace_back(&start_game_button);
    }
    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text(tr("menu.settings"));
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_push(SceneType::SETTINGS);
        });
        m_pending_enable.emplace_back(&button);
    }

    {
        auto& button = layout.add_child<Button>();

        button.set_background_image("texture/ui/button001.png",
                                    texture_manager);
        button.set_text(tr("menu.main.credits"));
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_push(SceneType::CREDITS);
        });
        m_pending_enable.emplace_back(&button);
    }
    {
        auto& exit_game = layout.add_child<Button>();
        exit_game.set_background_image("texture/ui/button001.png",
                                       texture_manager);
        exit_game.set_text(tr("menu.main.quit"));

        exit_game.set_clicked([this]() {
            m_scene.scene_manager().app().window().should_close_window();
        });
    }
    m_widgets.try_emplace("background", image.get());
    m_widgets.try_emplace("main menu layout", &layout);
    {
        constexpr float SCALE = 0.5f;
        auto& info_layout = image->add_child<ColumnLayout>();
        info_layout.set_anchor(Anchor::BOTTOM_LEFT);
        info_layout.set_spacing(10);
        info_layout.set_child_anchor(ColumnLayoutAnchor::LEFT);
        auto& player_name = info_layout.add_child<Label>();
        player_name.set_scale(SCALE);

        player_name.set_text(
            tr("menu.main.player_name",
               arg("name",
                   m_scene.scene_manager().app().argument().player.value_or(
                       "Unknown"))));

        auto& version = info_layout.add_child<Label>();
        version.set_scale(SCALE);
        std::string version_str = CUBED_VERSION;
#ifdef DEBUG_MODE
        version_str.append("-debug");
#else
        version_str.append("-release");
#endif
        version.set_text(
            tr("menu.main.cubed_version", arg("version", version_str)));
    }

    m_root_widget = std::move(image);
}

void MainMenuUIManager::on_re_enter() {
    for (Button* b : m_pending_enable) {
        b->set_enable(true);
    }
}

} // namespace Cubed