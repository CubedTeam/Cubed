#include "Cubed/scene/create_game_scene.hpp"

namespace Cubed {
CreateGameScene::CreateGameScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_ui(*this) {}
CreateGameScene::~CreateGameScene() {}

void CreateGameScene::update(float dt) { m_ui.update(dt); }
void CreateGameScene::render(Renderer& renderer) { m_ui.render(renderer); }
bool CreateGameScene::handle_event(const Event& e) {
    return m_ui.handle_event(e);
}
void CreateGameScene::on_enter() { m_ui.init(); }
void CreateGameScene::on_leave() {}
void CreateGameScene::on_re_enter() { m_ui.on_re_enter(); }
SceneManager& CreateGameScene::scene_manager() { return m_scene_manager; }

} // namespace Cubed