#include "Cubed/app.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/pasue_menu_ui_manager.hpp"
#include "Cubed/ui/rect.hpp"
namespace Cubed {
PauseMenuUIManager::PauseMenuUIManager(WorldScene& scene) : m_scene(scene) {}
void PauseMenuUIManager::init() {
    auto rect = std::make_unique<Rect>(nullptr);

    rect->set_fill(true);
    rect->set_color(Color::BLACK);
    rect->set_alpha(0.5f);
    rect->set_anchor(Anchor::TOP_LEFT);
    auto& renderer = m_scene.scene_manager().app().renderer();
    rect->set_window_size(renderer.window_width(), renderer.window_height());
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    auto& layout = rect->add_child<ColumnLayout>();
    layout.set_anchor(Anchor::CENTER);
    layout.set_spacing(20);
    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text("Back to Game");
        button.set_clicked([this]() { m_scene.set_pause(false); });
    }

    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text("Settings");
        button.set_clicked([this]() {
            m_scene.scene_manager().request_push(SceneType::SETTINGS);
        });
    }

    {
        auto& back_main = layout.add_child<Button>();

        back_main.set_background_image("texture/ui/button001.png",
                                       texture_manager);
        back_main.set_text("Return to Menu");
        back_main.set_clicked(
            [this]() { m_scene.scene_manager().request_pop(); });
    }

    m_root_widget = std::move(rect);
}

} // namespace Cubed