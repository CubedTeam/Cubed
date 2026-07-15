#include "Cubed/ui/ui_manager.hpp"

#include "Cubed/render/renderer.hpp"

namespace Cubed {

UIManager::~UIManager() {}

void UIManager::init() {}

void UIManager::update(float dt) { m_root_widget->update(dt); }

void UIManager::render(Renderer& renderer) {
    renderer.begin_render_ui();

    m_root_widget->render(renderer);

    renderer.end_render_ui();
}

Widget* UIManager::get_widget(std::string name) {

    auto it = m_widgets.find(name);
    if (it != m_widgets.end()) {
        return it->second;
    }
    return nullptr;
}

bool UIManager::handle_event(const Event& e) {
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
                   [this](const TextInputEvent& e) {
                       if (handle_text_input_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const WindowResizeEvent& e) {
                       handle_window_resize_event(e);
                       return false;
                   },
                   [](const FrameBufferResizeEvent&) { return false; }

        },
        e);
}
bool UIManager::handle_mouse_move_event(const MouseMoveEvent& e) {
    if (m_root_widget->handle_mouse_move_event(e)) {
        return true;
    }
    return false;
}
bool UIManager::handle_mouse_button_event(const MouseButtonEvent& e) {

    if (m_root_widget->handle_mouse_button_event(e)) {
        return true;
    }

    return false;
}
bool UIManager::handle_window_resize_event(const WindowResizeEvent& e) {

    if (m_root_widget->handle_window_resize_event(e)) {
        return true;
    }

    return false;
}
bool UIManager::handle_mouse_wheel_event(const MouseWheelEvent& e) {

    if (m_root_widget->handle_mouse_wheel_event(e)) {
        return true;
    }

    return false;
}
bool UIManager::handle_key_event(const KeyEvent& e) {

    if (m_root_widget->handle_key_event(e)) {
        return true;
    }

    return false;
}

bool UIManager::handle_text_input_event(const TextInputEvent& e) {
    if (m_root_widget->handle_text_input_event(e)) {
        return true;
    }
    return false;
}

} // namespace Cubed