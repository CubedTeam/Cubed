#pragma once
#include "Cubed/argument.hpp"
#include "Cubed/camera.hpp"
#include "Cubed/dev_panel.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/gameplay/network_server.hpp"
#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/pasue_menu_ui_manager.hpp"
#include "Cubed/ui/world_ui_manager.hpp"
namespace Cubed {
class SceneManager;
class WorldScene : public Scene {
public:
    WorldScene(const WorldScene&) = delete;
    WorldScene(WorldScene&&) = delete;
    WorldScene& operator=(const WorldScene&) = delete;
    WorldScene& operator=(WorldScene&&) = delete;

    WorldScene(SceneManager& scene_manager);
    ~WorldScene();

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;

    Camera& camera();
    SceneManager& scene_manager();
    ClientWorld& client_world();
    ServerWorld& server_world();
    bool pause() const;
    void set_pause(bool pause);

private:
    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;
    bool handle_window_resize_event(const WindowResizeEvent& e) override;
    bool handle_mouse_wheel_event(const MouseWheelEvent& e) override;
    bool handle_key_event(const KeyEvent& e) override;

    SceneManager& m_scene_manager;
    DevPanel m_dev_panel;
    Camera m_camera;
    NetworkServer m_server;
    std::shared_ptr<NetworkClient> m_client;
    ClientWorld m_client_world;
    bool m_paused = false;
    bool m_show_hud = true;
    bool m_show_dev_pannel = true;
    PauseMenuUIManager m_pasue_menu;
    WorldUIManager m_hud_ui;
    const Argument& m_argument;
};
} // namespace Cubed