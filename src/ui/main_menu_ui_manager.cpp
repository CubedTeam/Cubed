#include "Cubed/ui/main_menu_ui_manager.hpp"

#include "Cubed/app.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/main_menu_scene.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/ui/button.hpp"
namespace Cubed {

MainMenuUIManager::MainMenuUIManager(MainMenuScene& scene) : m_scene(scene) {}
MainMenuUIManager::~MainMenuUIManager() {}

void MainMenuUIManager::init() {

    auto start_game_button = std::make_unique<Button>(nullptr);

    auto& back = start_game_button->set_background<Image>();
    back.set_image("texture/ui/button001.png",
                   m_scene.scene_manager().app().texture_manager());
    auto& fore = start_game_button->set_foreground<Label>();
    fore.set_text("Start Game");
    fore.set_scale(0.6f);
    start_game_button->set_window_size(
        m_scene.scene_manager().app().renderer().window_width(),
        m_scene.scene_manager().app().renderer().window_height());
    start_game_button->set_clicked(
        [this]() { m_scene.scene_manager().request_push(SceneType::WORLD); });
    start_game_button->set_scale(3.0f);
    start_game_button->set_anchor(Anchor::CENTER);
    m_widgets.try_emplace("start game", std::move(start_game_button));
}

void MainMenuUIManager::update(float dt) {
    for (auto& w : m_widgets) {
        w.second->update(dt);
    }
}

void MainMenuUIManager::render(Renderer& renderer) {
    renderer.begin_render_ui();

    for (auto& widget : m_widgets) {
        widget.second->render(renderer);
    }

    renderer.end_render_ui();
}
bool MainMenuUIManager::handle_event(const Event& e) {
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
bool MainMenuUIManager::handle_mouse_move_event(const MouseMoveEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_mouse_move_event(e)) {
            return true;
        }
    }
    return false;
}
bool MainMenuUIManager::handle_mouse_button_event(const MouseButtonEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_mouse_button_event(e)) {
            return true;
        }
    }
    return false;
}
bool MainMenuUIManager::handle_window_resize_event(const WindowResizeEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_window_resize_event(e)) {
            return true;
        }
    }
    return false;
}
bool MainMenuUIManager::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_mouse_wheel_event(e)) {
            return true;
        }
    }
    return false;
}
bool MainMenuUIManager::handle_key_event(const KeyEvent& e) {
    for (auto& w : m_widgets) {
        if (w.second->handle_key_event(e)) {
            return true;
        }
    }

    return false;
}

} // namespace Cubed