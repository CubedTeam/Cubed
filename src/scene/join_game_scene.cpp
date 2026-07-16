#include "Cubed/scene/join_game_scene.hpp"

namespace Cubed {
JoinGameScene::JoinGameScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_ui(*this) {}

void JoinGameScene::update(float dt) { m_ui.update(dt); }
void JoinGameScene::render(Renderer& renderer) { m_ui.render(renderer); }
bool JoinGameScene::handle_event(const Event& e) {
    return m_ui.handle_event(e);
}
void JoinGameScene::on_enter() { m_ui.init(); }
void JoinGameScene::on_leave() {}
void JoinGameScene::on_re_enter() { m_ui.on_re_enter(); }
SceneManager& JoinGameScene::scene_manager() { return m_scene_manager; }
} // namespace Cubed