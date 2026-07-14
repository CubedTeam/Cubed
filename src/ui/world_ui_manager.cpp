#include "Cubed/ui/world_ui_manager.hpp"

#include "Cubed/app.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_scene.hpp"
namespace Cubed {

WorldUIManager::WorldUIManager(WorldScene& scene) : m_scene(scene) {}
WorldUIManager::~WorldUIManager() {}

void WorldUIManager::init() {
    auto crosshair = std::make_unique<Image>(nullptr);

    crosshair->set_image("texture/ui/0.png",
                         m_scene.scene_manager().app().texture_manager());
    auto& renderer = m_scene.scene_manager().app().renderer();
    crosshair->set_window_size(renderer.window_width(),
                               renderer.window_height());
    crosshair->set_anchor(Anchor::CENTER);
    crosshair->set_scale(3.0f);
    m_widgets.try_emplace("crosshair", crosshair.get());
    m_root_widget = std::move(crosshair);
}
void WorldUIManager::render(Renderer& renderer) {
    renderer.begin_render_ui();

    auto& widget = DebugCollector::get().get_widget();
    widget.render(renderer);
    m_root_widget->render(renderer);
    renderer.end_render_ui();
}

} // namespace Cubed