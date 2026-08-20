#include "Cubed/scene/world_select_scene.hpp"

namespace cubed {
WorldSelectScene::WorldSelectScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_ui(*this) {}

WorldSelectScene::~WorldSelectScene() {}

void WorldSelectScene::update(float dt) { m_ui.update(dt); }
void WorldSelectScene::render(Renderer& renderer) { m_ui.render(renderer); }
bool WorldSelectScene::handle_event(const Event& e) {
    return m_ui.handle_event(e);
}
void WorldSelectScene::on_enter() { m_ui.init(); }
void WorldSelectScene::on_leave() {}
SceneManager& WorldSelectScene::scene_manager() { return m_scene_manager; }

} // namespace cubed
