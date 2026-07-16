#pragma once

#include "Cubed/ui/button.hpp"
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
    void on_re_enter();

private:
    WorldScene& m_scene;
    std::vector<Button*> m_pending_enable;
};
} // namespace Cubed