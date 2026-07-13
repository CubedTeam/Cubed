#pragma once

#include "Cubed/input/event.hpp"
#include "Cubed/ui/widget.hpp"

#include <memory>
#include <unordered_map>
namespace Cubed {
class Renderer;
class MainMenuScene;
class MainMenuUIManager {
public:
    MainMenuUIManager(MainMenuScene& m_scene);
    MainMenuUIManager(const MainMenuUIManager&) = delete;
    MainMenuUIManager(MainMenuUIManager&&) = delete;
    MainMenuUIManager& operator=(const MainMenuUIManager&) = delete;
    MainMenuUIManager& operator=(MainMenuUIManager&&) = delete;
    ~MainMenuUIManager();

    void init();
    void render(Renderer& renderer);
    void update(float dt);

    bool handle_event(const Event& e);

private:
    MainMenuScene& m_scene;
    std::unique_ptr<Widget> m_root_widget;
    std::unordered_map<std::string, Widget*> m_widgets;
    bool handle_mouse_move_event(const MouseMoveEvent& e);
    bool handle_mouse_button_event(const MouseButtonEvent& e);
    bool handle_window_resize_event(const WindowResizeEvent& e);
    bool handle_mouse_wheel_event(const MouseWheelEvent& e);
    bool handle_key_event(const KeyEvent& e);
};
} // namespace Cubed