#pragma once

#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/create_game_ui.hpp"
namespace cubed {
class SceneManager;
class CreateGameScene : public Scene {
public:
    CreateGameScene(SceneManager& scene_manager);
    ~CreateGameScene();

    CreateGameScene(const CreateGameScene&) = delete;
    CreateGameScene(CreateGameScene&&) = delete;
    CreateGameScene& operator=(const CreateGameScene&) = delete;
    CreateGameScene& operator=(CreateGameScene&&) = delete;

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;
    void on_re_enter() override;
    SceneManager& scene_manager();

private:
    SceneManager& m_scene_manager;
    CreateGameUI m_ui;
};
} // namespace cubed
