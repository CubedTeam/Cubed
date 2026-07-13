#include "Cubed/scene/main_menu_scene.hpp"

namespace Cubed {
MainMenuScene::MainMenuScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_ui_manager(*this) {}
MainMenuScene::~MainMenuScene() {}

void MainMenuScene::update(float dt) { m_ui_manager.update(dt); }
void MainMenuScene::render(Renderer& renderer) {
    m_ui_manager.render(renderer);
}
bool MainMenuScene::handle_event(const Event& e) {
    if (m_ui_manager.handle_event(e)) {
        return true;
    }
    return false;
}
void MainMenuScene::on_enter() { m_ui_manager.init(); }
void MainMenuScene::on_leave() {}
SceneManager& MainMenuScene::scene_manager() { return m_scene_manager; }
} // namespace Cubed