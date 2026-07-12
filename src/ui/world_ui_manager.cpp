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
    auto crosshair = std::make_unique<Image>();

    crosshair->set_image("texture/ui/0.png",
                         m_scene.scene_manager().app().texture_manager());
    auto& renderer = m_scene.scene_manager().app().renderer();
    crosshair
        ->set_position(renderer.window_width() / 2 + crosshair->width() / 2,
                       renderer.window_height() / 2 + crosshair->height() / 2)
        .set_scale(3.0f);
    m_widgets.try_emplace("crosshair", std::move(crosshair));
}
void WorldUIManager::update(float) {}
void WorldUIManager::render(Renderer& renderer) {
    renderer.begin_render_ui();

    auto& widget = DebugCollector::get().get_widget();
    widget.render(renderer);
    for (auto& widget : m_widgets) {
        widget.second->render(renderer);
    }

    renderer.end_render_ui();
}
bool WorldUIManager::handle_event(const Event& e) {
    return std::visit(
        Overloaded{[](const MouseMoveEvent&) { return false; },
                   [this](const MouseButtonEvent& e) {
                       if (handle_mouse_button_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const MouseWheelEvent& e) {
                       if (handle_mouse_wheel_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const KeyEvent& e) {
                       if (handle_key_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [](const TextInputEvent&) { return false; },
                   [this](const WindowResizeEvent& e) {
                       handle_window_resize_event(e);
                       return false;
                   },
                   [](const FrameBufferResizeEvent&) { return false; }

        },
        e);
}

bool WorldUIManager::handle_mouse_button_event(const MouseButtonEvent&) {
    return false;
}
bool WorldUIManager::handle_window_resize_event(const WindowResizeEvent& e) {
    auto it = m_widgets.find("crosshair");
    if (it != m_widgets.end()) {
        auto* crosshair = dynamic_cast<Image*>(it->second.get());
        if (crosshair) {
            crosshair->set_position(e.width / 2 + crosshair->width() / 2,
                                    e.height / 2 + crosshair->height() / 2);
        }
    }
    return false;
}
bool WorldUIManager::handle_mouse_wheel_event(const MouseWheelEvent&) {
    return false;
}
bool WorldUIManager::handle_key_event(const KeyEvent&) { return false; }

} // namespace Cubed