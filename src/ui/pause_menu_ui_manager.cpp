#include "Cubed/app.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/ui/button.hpp"
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
    auto& back_main = rect->add_child<Button>();

    auto& bg = back_main.set_background<Image>();
    bg.set_image("texture/ui/button001.png",
                 m_scene.scene_manager().app().texture_manager());
    auto& fg = back_main.set_foreground<Label>();
    fg.set_text("return to main menu");
    fg.set_scale(0.5f);
    back_main.set_scale(5.0f);
    back_main.set_anchor(Anchor::CENTER);
    back_main.set_clicked([this]() { m_scene.scene_manager().request_pop(); });
    m_root_widget = std::move(rect);
}

bool PauseMenuUIManager::handle_key_event(const KeyEvent& e) {
    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
        m_scene.set_pause(false);
        return true;
    }
    return UIManager::handle_key_event(e);
}

} // namespace Cubed