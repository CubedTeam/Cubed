#pragma once
#include "Cubed/scene/scene.hpp"
#include "Cubed/ui/screenshot_ui.hpp"
namespace cubed {
class SceneManager;
class ScreenshotScene : public Scene {
public:
    ScreenshotScene(SceneManager& scene_manager);
    ~ScreenshotScene();

    ScreenshotScene(const ScreenshotScene&) = delete;
    ScreenshotScene(ScreenshotScene&&) = delete;
    ScreenshotScene& operator=(const ScreenshotScene&) = delete;
    ScreenshotScene& operator=(ScreenshotScene&&) = delete;

    void update(float dt) override;
    void render(Renderer& renderer) override;
    bool handle_event(const Event& e) override;
    void on_enter() override;
    void on_leave() override;
    void on_re_enter() override;

    SceneManager& scene_manager();

private:
    SceneManager& m_scene_manager;
    ScreenshotUI m_ui;
};
} // namespace cubed
