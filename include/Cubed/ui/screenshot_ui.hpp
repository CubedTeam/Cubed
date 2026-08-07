#pragma once

#include "Cubed/ui/image.hpp"
#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class Texture;
class ScreenshotScene;
class ScreenshotUI : public UIManager {
public:
    ScreenshotUI(ScreenshotScene& m_scene);
    ~ScreenshotUI();

    ScreenshotUI(const ScreenshotUI&) = delete;
    ScreenshotUI(ScreenshotUI&&) = delete;
    ScreenshotUI& operator=(const ScreenshotUI&) = delete;
    ScreenshotUI& operator=(ScreenshotUI&&) = delete;

    void init() override;

    void update_layout(int width, int height);

    void open_lightbox(const Texture* image);

private:
    struct ImageInfo {
        const Texture* texture = nullptr;
        int width = 0;
        int height = 0;
    };

    ScreenshotScene& m_scene;
    std::vector<ImageInfo> m_image_infos;
    Widget* m_lightbox = nullptr;
    Image* m_lightbox_image = nullptr;
    bool handle_window_resize_event(const WindowResizeEvent& e) override;
    bool handle_key_event(const KeyEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;
    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_wheel_event(const MouseWheelEvent& e) override;
};
} // namespace Cubed