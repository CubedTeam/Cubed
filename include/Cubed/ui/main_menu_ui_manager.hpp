#pragma once

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

private:
    MainMenuScene& m_scene;
};
} // namespace Cubed