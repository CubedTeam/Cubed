#pragma once

#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/main_menu_ui_manager.hpp"
namespace Cubed {
class SceneManager;
class MainMenuScene : public Scene {
public:
    MainMenuScene(const MainMenuScene&) = delete;
    MainMenuScene(MainMenuScene&&) = delete;
    MainMenuScene& operator=(const MainMenuScene&) = delete;
    MainMenuScene& operator=(MainMenuScene&&) = delete;
    MainMenuScene(SceneManager& scene_manager);
    ~MainMenuScene();

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;
    void on_re_enter() override;
    SceneManager& scene_manager();

private:
    SceneManager& m_scene_manager;
    MainMenuUIManager m_ui_manager;
};
} // namespace Cubed