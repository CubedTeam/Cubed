#include "Cubed/scene/host_game_scene.hpp"

namespace Cubed {
HostGameScene::HostGameScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_ui(*this) {}
HostGameScene::~HostGameScene() {}

void HostGameScene::update(float dt) { m_ui.update(dt); }
void HostGameScene::render(Renderer& renderer) { m_ui.render(renderer); }
bool HostGameScene::handle_event(const Event& e) {
    return m_ui.handle_event(e);
}
void HostGameScene::on_enter() { m_ui.init(); }
void HostGameScene::on_leave() {}
void HostGameScene::on_re_enter() { m_ui.on_re_enter(); }
SceneManager& HostGameScene::scene_manager() { return m_scene_manager; }

} // namespace Cubed