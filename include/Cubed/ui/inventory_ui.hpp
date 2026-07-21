#pragma once

#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class WorldScene;
class InventoryUI : public UIManager {
public:
    InventoryUI(WorldScene& m_scene);

    void init() override;
    void on_re_enter();

private:
    WorldScene& m_scene;
};
} // namespace Cubed