#pragma once

#include "Cubed/input/event.hpp"
#include "Cubed/ui/widget.hpp"

#include <string>
#include <unordered_map>
namespace Cubed {
class WorldScene;
class WorldUIManager {
public:
    WorldUIManager(const WorldUIManager&) = delete;
    WorldUIManager(WorldUIManager&&) = delete;
    WorldUIManager& operator=(const WorldUIManager&) = delete;
    WorldUIManager& operator=(WorldUIManager&&) = delete;

    WorldUIManager(WorldScene& scene);
    ~WorldUIManager();

    void init();
    void update(float dt);
    void render(Renderer& renderer);
    bool handle_event(const Event& e);

private:
    WorldScene& m_scene;
    std::unordered_map<std::string, std::unique_ptr<Widget>> m_widgets;

    bool handle_mouse_button_event(const MouseButtonEvent& e);
    bool handle_window_resize_event(const WindowResizeEvent& e);
    bool handle_mouse_wheel_event(const MouseWheelEvent& e);
    bool handle_key_event(const KeyEvent& e);
};
} // namespace Cubed