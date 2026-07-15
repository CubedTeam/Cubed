#pragma once

#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/join_game_ui.hpp"
namespace Cubed {
class SceneManager;
class JoinGameScene : public Scene {
public:
    JoinGameScene(const JoinGameScene&) = delete;
    JoinGameScene(JoinGameScene&&) = delete;
    JoinGameScene& operator=(const JoinGameScene&) = delete;
    JoinGameScene& operator=(JoinGameScene&&) = delete;

    JoinGameScene(SceneManager& scene_manager);

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;
    void on_re_enter() override;
    SceneManager& scene_manager();

private:
    SceneManager& m_scene_manager;
    JoinGameUI m_ui;
};
} // namespace Cubed