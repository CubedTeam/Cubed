#pragma once

#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/world_select_ui.hpp"
namespace cubed {
class SceneManager;
class WorldSelectScene : public Scene {
public:
    WorldSelectScene(SceneManager& scene_manager);
    ~WorldSelectScene();

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;
    SceneManager& scene_manager();

private:
    SceneManager& m_scene_manager;
    WorldSelectUI m_ui;
};
} // namespace cubed
