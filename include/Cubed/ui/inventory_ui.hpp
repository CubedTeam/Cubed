#pragma once

#include "Cubed/ui/item_slot.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class WorldScene;
class InventoryUI : public UIManager {
public:
    InventoryUI(WorldScene& m_scene);

    void init() override;
    void on_re_enter();
    void update(float dt) override;

private:
    WorldScene& m_scene;
    std::vector<ItemSlot*> m_slots;
    Label* m_item_info = nullptr;
    void update_item_info();
};
} // namespace Cubed