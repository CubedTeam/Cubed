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
    m_widgets.try_emplace("crosshair", std::move(crosshair));
}
void WorldUIManager::update(float dt) {

    for (auto& w : m_widgets) {
        w.second->update(dt);
    }
}
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
        Overloaded{[this](const MouseMoveEvent& e) {
                       if (handle_mouse_move_event(e)) {
                           return true;
                       }
                       return false;
                   },
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
bool WorldUIManager::handle_mouse_move_event(const MouseMoveEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_mouse_move_event(e)) {
            return true;
        }
    }
    return false;
}
bool WorldUIManager::handle_mouse_button_event(const MouseButtonEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_mouse_button_event(e)) {
            return true;
        }
    }
    return false;
}
bool WorldUIManager::handle_window_resize_event(const WindowResizeEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_window_resize_event(e)) {
            return true;
        }
    }
    return false;
}
bool WorldUIManager::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_mouse_wheel_event(e)) {
            return true;
        }
    }
    return false;
}
bool WorldUIManager::handle_key_event(const KeyEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_key_event(e)) {
            return true;
        }
    }

    return false;
}

} // namespace Cubed