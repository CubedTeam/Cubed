#pragma once

#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class SettingsScene;
class SettingsUI : public UIManager {
public:
    SettingsUI(SettingsScene& scene);
    void init() override;

private:
    bool handle_key_event(const KeyEvent& e) override;
    SettingsScene& m_scene;
};

} // namespace Cubed