#pragma once

#include "Cubed/ui/ui_manager.hpp"
namespace cubed {

class CreditsScene;

class CreditsUI : public UIManager {
public:
    CreditsUI(CreditsScene& m_scene);
    void init() override;
    void update_layout(int width, int height);

private:
    bool handle_key_event(const KeyEvent& e) override;
    bool handle_window_resize_event(const WindowResizeEvent& e) override;
    CreditsScene& m_scene;
};
} // namespace cubed
