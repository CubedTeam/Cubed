#pragma once

#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/credits_ui.hpp"
namespace Cubed {
class SceneManager;
class CreditsScene : public Scene {
public:
    CreditsScene(SceneManager& scene_manager);
    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_re_enter() override;
    SceneManager& scene_manager();

private:
    SceneManager& m_scene_manager;
    CreditsUI m_ui;
};
} // namespace Cubed