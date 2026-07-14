#include "Cubed/scene/credits_scene.hpp"

namespace Cubed {
CreditsScene::CreditsScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_ui(*this) {}

void CreditsScene::update(float dt) { m_ui.update(dt); }
void CreditsScene::render(Renderer& renderer) { m_ui.render(renderer); }
bool CreditsScene::handle_event(const Event& e) {
    if (m_ui.handle_event(e)) {
        return true;
    }
    return false;
}
void CreditsScene::on_enter() { m_ui.init(); }
SceneManager& CreditsScene::scene_manager() { return m_scene_manager; }

} // namespace Cubed