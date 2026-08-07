#pragma once

#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class ScreenshotScene;
class ScreenshotUI : public UIManager {
public:
    ScreenshotUI(ScreenshotScene& m_scene);
    ~ScreenshotUI();

    ScreenshotUI(const ScreenshotUI&) = delete;
    ScreenshotUI(ScreenshotUI&&) = delete;
    ScreenshotUI& operator=(const ScreenshotUI&) = delete;
    ScreenshotUI& operator=(ScreenshotUI&&) = delete;

    void init();

    void update_layout(int width, int height);

private:
    ScreenshotScene& m_scene;
};
} // namespace Cubed