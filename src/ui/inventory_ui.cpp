#include "Cubed/ui/inventory_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/image.hpp"

namespace Cubed {
InventoryUI::InventoryUI(WorldScene& scene) : m_scene(scene) {}

void InventoryUI::init() {
    auto back = std::make_unique<Rect>(nullptr);

    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    back->set_anchor(Anchor::TOP_LEFT);
    back->set_color(Color::BLACK).set_alpha(0.7f);
    back->set_fill_parent(true);

    auto& column = back->add_child<ColumnLayout>();
    column.set_anchor(Anchor::CENTER);
    column.set_child_anchor(ColumnLayoutAnchor::LEFT);
    auto& block_textures = texture_manager.get_item_textures();
    auto sum = block_textures.size();
    {
        auto& row_layout = column.add_child<RowLayout>();
        auto row = &row_layout;
        for (size_t i = 1; i < sum; ++i) {
            if ((i - 1) % 10 == 0) {
                auto& r = column.add_child<RowLayout>();
                row = &r;
            }
            auto& slot = row->add_child<ItemSlot>();
            slot.set_default_background(texture_manager);
            slot.set_scale(5.0f);
            slot.set_item(i, block_textures[i].get());
            m_slots.emplace_back(&slot);
        }
    }
    {
        auto& label = back->add_child<Label>();
        auto rect = std::make_unique<Rect>(&label);
        rect->set_fill_parent(true);
        rect->set_color(Color::BLACK);
        label.set_background(std::move(rect));
        label.set_anchor(Anchor::FOLLOW_MOUSE);
        label.set_scale(0.8f);
        label.set_visible(false);
        label.set_offset({20, -20});
        m_item_info = &label;
    }
    {
        auto& spec = column.add_child<Rect>();
        spec.set_color(Color::GRAY).set_fill_width(true).set_height(15.0f);
    }
    {
        auto& player = m_scene.client_world().get_player();
        auto hotbar = player.get_hotbar();
        auto& row = column.add_child<RowLayout>();

        for (auto& h : hotbar) {
            auto& item = row.add_child<ItemSlot>();
            item.set_default_background(texture_manager);
            item.set_scale(5.0f);
            if (h.type == 0) {
                item.set_item(0, nullptr);
            } else {
                item.set_item(h.type, block_textures[h.type].get());
            }
            m_hotbar.emplace_back(&item);
        }
    }
    {
        auto& image = back->add_child<Image>();
        image.set_texture(nullptr, true);
        image.set_scale(0.25f);
        image.set_anchor(Anchor::FOLLOW_MOUSE).set_offset({0, 0});
        image.set_visible(false);
        m_selected_image = &image;
    }
    m_root_widget = std::move(back);
}
void InventoryUI::on_re_enter() {}
void InventoryUI::update(float dt) {
    UIManager::update(dt);
    update_item_info();
}

void InventoryUI::update_item_info() {
    auto show_item_info = [this](ItemSlot* slot) {
        if (slot && !m_selected_image->has_texture()) {
            auto type = slot->block();
            if (type != 0) {
                m_item_info->set_text(BlockManager::local_name(type))
                    .set_visible(true);
                return true;
            }
        }
        return false;
    };
    {
        auto slot = get_hovered_slot();
        if (show_item_info(slot)) {
            return;
        }
    }
    {
        auto [slot, _] = get_hovered_hotbar_slot();
        if (show_item_info(slot)) {
            return;
        }
    }
    m_item_info->set_visible(false);
}

bool InventoryUI::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {

        auto& texture_manager = m_scene.scene_manager().app().texture_manager();
        auto& blocks = texture_manager.get_item_textures();
        if (!m_selected_image->has_texture()) {

            {
                auto slot = get_hovered_slot();
                if (slot) {
                    m_selected_block = slot->block();
                    if (m_selected_block != 0) {
                        m_selected_image->set_texture(
                            blocks[m_selected_block].get(), true);
                        m_selected_image->set_visible(true);
                        return true;
                    }
                }
            }
            {
                auto [slot, pos] = get_hovered_hotbar_slot();
                if (slot) {
                    m_selected_block = slot->block();
                    if (m_selected_block != 0) {
                        m_selected_image->set_texture(
                            blocks[m_selected_block].get(), true);
                        m_hotbar[pos]->set_item(0, nullptr);
                        auto& player = m_scene.client_world().get_player();
                        player.set_hotbar(pos, {0, 0});
                        m_selected_image->set_visible(true);
                        return true;
                    }
                }
            }
        }
        if (m_selected_image->has_texture()) {
            m_selected_image->set_texture(nullptr, true);
            m_selected_image->set_visible(false);
            auto [slot, pos] = get_hovered_hotbar_slot();
            if (slot) {
                auto& player = m_scene.client_world().get_player();
                player.set_hotbar(pos, {m_selected_block, 1});
                if (m_selected_block != 0) {
                    m_hotbar[pos]->set_item(m_selected_block,
                                            blocks[m_selected_block].get());
                }
            }
            return true;
        }
    }
    return UIManager::handle_mouse_button_event(e);
}
ItemSlot* InventoryUI::get_hovered_slot() {
    for (auto& slot : m_slots) {
        if (slot->hovered()) {

            return slot;
        }
    }
    return nullptr;
}
std::pair<ItemSlot*, size_t> InventoryUI::get_hovered_hotbar_slot() {
    for (size_t i = 0; i < ClientPlayer::HOTBAR_SUM; ++i) {
        auto& slot = m_hotbar[i];
        if (slot->hovered()) {

            return {slot, i};
        }
    }
    return {nullptr, 0};
}
} // namespace Cubed