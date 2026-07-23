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
    std::vector<ItemSlot*> m_hotbar;
    Label* m_item_info = nullptr;

    Image* m_selected_image = nullptr;
    BlockType m_selected_block = 0;
    void update_item_info();
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;

    ItemSlot* get_hovered_slot();
    std::pair<ItemSlot*, size_t> get_hovered_hotbar_slot();
};
} // namespace Cubed