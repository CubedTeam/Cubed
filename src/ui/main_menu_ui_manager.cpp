#include "Cubed/ui/main_menu_ui_manager.hpp"

#include "Cubed/app.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/main_menu_scene.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
namespace Cubed {

MainMenuUIManager::MainMenuUIManager(MainMenuScene& scene) : m_scene(scene) {}
MainMenuUIManager::~MainMenuUIManager() {}

void MainMenuUIManager::init() {

    auto image = std::make_unique<Image>(nullptr);

    image->set_fill(true);
    image->set_anchor(Anchor::TOP_LEFT);
    image->set_image("texture/ui/background.png",
                     m_scene.scene_manager().app().texture_manager());
    auto& renderer = m_scene.scene_manager().app().renderer();
    image->set_window_size(renderer.window_width(), renderer.window_height());

    auto& layout = image->add_child<ColumnLayout>();
    layout.set_spacing(20);
    layout.set_anchor(Anchor::CENTER);
    layout.set_window_size(renderer.window_width(), renderer.window_height());
    {
        auto& start_game_button = layout.add_child<Button>();

        auto& back = start_game_button.set_background<Image>();
        back.set_image("texture/ui/button001.png",
                       m_scene.scene_manager().app().texture_manager());
        auto& fore = start_game_button.set_foreground<Label>();
        fore.set_text("Start Game");
        fore.set_scale(0.6f);
        start_game_button.set_clicked([this]() {
            m_scene.scene_manager().request_push(SceneType::WORLD);
        });
        start_game_button.set_scale(4.0f);
    }

    auto& exit_game = layout.add_child<Button>();

    {
        auto& back = exit_game.set_background<Image>();
        back.set_image("texture/ui/button001.png",
                       m_scene.scene_manager().app().texture_manager());
        auto& fore = exit_game.set_foreground<Label>();
        fore.set_scale(0.6f).set_text("Exit");
        exit_game.set_scale(4.0f);
        exit_game.set_clicked([this]() {
            m_scene.scene_manager().app().window().should_close_window();
        });
    }
    m_widgets.try_emplace("background", image.get());
    m_widgets.try_emplace("main menu layout", &layout);
    m_root_widget = std::move(image);
}

} // namespace Cubed