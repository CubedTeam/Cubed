#pragma once
#include "Cubed/argument.hpp"
#include "Cubed/camera.hpp"
#include "Cubed/dev_panel.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/scene/scene.hpp"
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

private:
    SceneManager& m_scene_manager;
    DevPanel m_dev_panel;
    Camera m_camera;
    std::shared_ptr<NetworkClient> m_client;
    ClientWorld m_client_world;
    WorldUIManager m_ui_manager;
    const Argument& m_argument;
};
} // namespace Cubed