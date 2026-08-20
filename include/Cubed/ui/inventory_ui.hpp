#pragma once

#include "Cubed/gameplay/item_stack.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/item_slot.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/ui_manager.hpp"
namespace cubed {
class WorldScene;
class InventoryUI : public UIManager {
public:
    InventoryUI(WorldScene& m_scene);

    void init() override;
    void on_re_enter();
    void update(float dt) override;

    void refresh_hotbar_and_backpack();

private:
    WorldScene& m_scene;

    std::vector<ItemSlot*> m_creative_slots;
    std::vector<ItemSlot*> m_hotbar;
    std::vector<ItemSlot*> m_backpack_slots;

    ColumnLayout* m_creative = nullptr;
    ColumnLayout* m_backpack = nullptr;

    Label* m_item_info = nullptr;

    Image* m_selected_image = nullptr;

    std::optional<ItemStack> m_selected;
    std::optional<size_t> m_from;

    int m_current = 0;

    void update_item_info();
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;

    ItemSlot* get_hovered_creative_slot();
    std::pair<ItemSlot*, size_t> get_hovered_hotbar_slot();
    std::pair<ItemSlot*, size_t> get_hovered_backpack_slot();
};
} // namespace cubed
