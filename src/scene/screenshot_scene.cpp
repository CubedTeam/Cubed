#include "Cubed/scene/screenshot_scene.hpp"

namespace cubed {
ScreenshotScene::ScreenshotScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_ui(*this) {}

ScreenshotScene::~ScreenshotScene() {}

void ScreenshotScene::update(float dt) { m_ui.update(dt); }
void ScreenshotScene::render(Renderer& renderer) { m_ui.render(renderer); }
bool ScreenshotScene::handle_event(const Event& e) {
    return m_ui.handle_event(e);
}
void ScreenshotScene::on_enter() { m_ui.init(); }
void ScreenshotScene::on_leave() {}
void ScreenshotScene::on_re_enter() {}

SceneManager& ScreenshotScene::scene_manager() { return m_scene_manager; }

} // namespace cubed
