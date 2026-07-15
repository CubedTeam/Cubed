#pragma once

#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/host_game_ui.hpp"
namespace Cubed {
class SceneManager;
class HostGameScene : public Scene {
public:
    HostGameScene(SceneManager& scene_manager);
    ~HostGameScene();

    HostGameScene(const HostGameScene&) = delete;
    HostGameScene(HostGameScene&&) = delete;
    HostGameScene& operator=(const HostGameScene&) = delete;
    HostGameScene& operator=(HostGameScene&&) = delete;

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;
    void on_re_enter() override;
    SceneManager& scene_manager();

private:
    SceneManager& m_scene_manager;
    HostGameUI m_ui;
};
} // namespace Cubed