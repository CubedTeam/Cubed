#pragma once

#include "Cubed/ui/button.hpp"
#include "Cubed/ui/ui_manager.hpp"

namespace Cubed {
class Renderer;
class MainMenuScene;
class MainMenuUIManager : public UIManager {
public:
    MainMenuUIManager(MainMenuScene& m_scene);
    MainMenuUIManager(const MainMenuUIManager&) = delete;
    MainMenuUIManager(MainMenuUIManager&&) = delete;
    MainMenuUIManager& operator=(const MainMenuUIManager&) = delete;
    MainMenuUIManager& operator=(MainMenuUIManager&&) = delete;
    ~MainMenuUIManager();

    void init() override;
    void on_re_enter();

private:
    MainMenuScene& m_scene;
    std::vector<Button*> m_pending_enable;
};
} // namespace Cubed