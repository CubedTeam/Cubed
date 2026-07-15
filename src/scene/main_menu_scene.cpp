#include "Cubed/scene/main_menu_scene.hpp"

#include "Cubed/app.hpp"
#include "Cubed/scene/scene_manager.hpp"

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
void MainMenuScene::on_re_enter() {
    m_ui_manager.on_re_enter();
    auto width = m_scene_manager.app().renderer().window_width();
    auto height = m_scene_manager.app().renderer().window_height();

    handle_event(
        WindowResizeEvent{static_cast<int>(width), static_cast<int>(height)});
}
SceneManager& MainMenuScene::scene_manager() { return m_scene_manager; }
} // namespace Cubed