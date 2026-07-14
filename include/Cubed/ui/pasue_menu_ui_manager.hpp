#pragma once

#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class WorldScene;
class PauseMenuUIManager : public UIManager {
public:
    PauseMenuUIManager(const PauseMenuUIManager&) = delete;
    PauseMenuUIManager(PauseMenuUIManager&&) = delete;
    PauseMenuUIManager& operator=(const PauseMenuUIManager&) = delete;
    PauseMenuUIManager& operator=(PauseMenuUIManager&&) = delete;
    PauseMenuUIManager(WorldScene& scene);
    void init() override;

private:
    WorldScene& m_scene;
};
} // namespace Cubed