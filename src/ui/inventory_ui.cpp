#include "Cubed/ui/inventory_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/gameplay/item_manager.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/image.hpp"

namespace {
constexpr int SELECTED_ITEM_SIZE = 128;
}

namespace cubed {
InventoryUI::InventoryUI(WorldScene& scene) : m_scene(scene) {}

void InventoryUI::init() {
    auto back = std::make_unique<Rect>(nullptr);

    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    back->set_anchor(Anchor::TOP_LEFT);
    back->set_color(Color::BLACK).set_alpha(0.7f);
    back->set_fill_parent(true);

    auto& creative_column = back->add_child<ColumnLayout>();
    creative_column.set_anchor(Anchor::CENTER);
    creative_column.set_child_anchor(ColumnLayoutAnchor::LEFT);
    auto& item_textures = texture_manager.get_item_textures();

    {
        auto& row_layout = creative_column.add_child<RowLayout>();
        auto row = &row_layout;
        size_t i = 0;
        for (auto& [id, texture] : item_textures) {
            if (id == 0) {
                continue;
            }
            if (i % 10 == 0) {
                auto& r = creative_column.add_child<RowLayout>();
                row = &r;
            }
            auto& slot = row->add_child<ItemSlot>();
            slot.set_default_background(texture_manager);
            slot.set_scale(5.0f);

            slot.set_item(ItemStack{id, 1}, texture.get());
            m_creative_slots.emplace_back(&slot);
            ++i;
        }
    }
    m_creative = &creative_column;

    {
        auto& spec = creative_column.add_child<Rect>();
        spec.set_color(Color::GRAY).set_fill_width(true).set_height(15.0f);
    }

    auto& player = m_scene.client_world().get_player();

    {
        auto backpack = player.get_backpack();
        auto& backpack_column = back->add_child<ColumnLayout>();
        m_backpack = &backpack_column;
        backpack_column.set_anchor(Anchor::CENTER);
        backpack_column.set_child_anchor(ColumnLayoutAnchor::LEFT);

        auto& row_layout = backpack_column.add_child<RowLayout>();
        auto row = &row_layout;
        for (size_t i = 0; i < backpack.size(); ++i) {
            if (i % 10 == 0) {
                auto& r = backpack_column.add_child<RowLayout>();
                row = &r;
            }
            auto& slot = row->add_child<ItemSlot>();
            slot.set_default_background(texture_manager);
            slot.set_scale(5.0f);
            if (backpack[i]) {
                auto texture = item_textures.find(backpack[i]->item);
                if (texture == item_textures.end()) {
                    slot.set_item(backpack[i],
                                  item_textures.begin()->second.get());
                } else {
                    slot.set_item(backpack[i], texture->second.get());
                }

            } else {
                slot.set_item(backpack[i], nullptr);
            }

            m_backpack_slots.emplace_back(&slot);
        }
        m_backpack->set_visible(false);
    }

    {

        auto hotbar = player.get_hotbar();
        auto& row = back->add_child<RowLayout>();

        for (auto& h : hotbar) {
            auto& item = row.add_child<ItemSlot>();
            item.set_default_background(texture_manager);
            item.set_scale(5.0f);
            if (!h) {
                item.set_item(std::nullopt, nullptr);
            } else {
                auto it = item_textures.find(h->item);
                ASSERT(it != item_textures.end());
                item.set_item(h, it->second.get());
            }
            m_hotbar.emplace_back(&item);
        }
        row.set_anchor(Anchor::BOTTOM_CENTER);
        row.set_offset({0, -150});
        row.layout();
        {
            auto& button = back->add_child<Button>();
            button.set_background_image("cubed/textures/ui/slot_button.png",
                                        texture_manager);
            button.set_height(ItemSlot::DEFAULT_HEIGHT)
                .set_width(ItemSlot::DEFAULT_WIDTH)
                .set_scale(5.0f);
            button.set_anchor(Anchor::BOTTOM_CENTER);
            button.set_offset({row.width() / 2 + button.width() + 10, -150});
            auto& front = button.add_child<Image>();
            front.set_fill_parent(true);
            front.set_image("cubed/textures/ui/backpack.png", texture_manager,
                            false);
            button.set_clicked([this]() {
                if (m_current == 0) {
                    m_current = 1;
                    if (m_backpack) {
                        m_backpack->set_visible(true);
                    }
                    if (m_creative) {
                        m_creative->set_visible(false);
                    }
                } else {
                    m_current = 0;
                    if (m_backpack) {
                        m_backpack->set_visible(false);
                    }
                    if (m_creative) {
                        m_creative->set_visible(true);
                    }
                }
            });
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
        auto& image = back->add_child<Image>();

        image.set_width(SELECTED_ITEM_SIZE);
        image.set_height(SELECTED_ITEM_SIZE);

        image.set_anchor(Anchor::FOLLOW_MOUSE).set_offset({0, 0});
        image.set_visible(false);
        m_selected_image = &image;
    }
    m_root_widget = std::move(back);
}
void InventoryUI::on_re_enter() {}
void InventoryUI::update(float dt) {
    refresh_hotbar_and_backpack();
    UIManager::update(dt);
    update_item_info();
}

void InventoryUI::refresh_hotbar_and_backpack() {
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    const auto& item_textures = texture_manager.get_item_textures();
    auto& player = m_scene.client_world().get_player();
    {
        auto hotbar = player.get_hotbar();

        for (size_t i = 0; i < hotbar.size(); ++i) {
            if (!hotbar[i]) {
                if (m_hotbar[i]->id()) {
                    m_hotbar[i]->set_item(std::nullopt, nullptr);
                }
                continue;
            }

            if (m_hotbar[i]->stack() == hotbar[i]) {
                continue;
            }

            auto it = item_textures.find(hotbar[i]->item);
            ASSERT(it != item_textures.end());
            m_hotbar[i]->set_item(hotbar[i], it->second.get());
        }
    }
    {
        auto backpack = player.get_backpack();
        for (size_t i = 0; i < backpack.size(); ++i) {
            if (!backpack[i]) {
                if (m_backpack_slots[i]->id()) {
                    m_backpack_slots[i]->set_item(std::nullopt, nullptr);
                }
                continue;
            }

            if (m_backpack_slots[i]->stack() == backpack[i]) {
                continue;
            }

            auto it = item_textures.find(backpack[i]->item);
            ASSERT(it != item_textures.end());
            m_backpack_slots[i]->set_item(backpack[i], it->second.get());
        }
    }
}

void InventoryUI::update_item_info() {
    auto show_item_info = [this](ItemSlot* slot) {
        if (slot && !m_selected_image->has_texture()) {

            auto type = slot->id();

            if (type && type != 0) {
                auto data = ItemManager::get(*type);
                m_item_info->set_text(data.local_name).set_visible(true);
                return true;
            }
        }
        return false;
    };
    if (m_current == 0) {
        auto slot = get_hovered_creative_slot();
        if (show_item_info(slot)) {
            return;
        }
    } else {
        auto [slot, _] = get_hovered_backpack_slot();
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
        auto& item_textures = texture_manager.get_item_textures();
        if (!m_selected_image->has_texture()) {

            if (m_current == 0) {
                // Creative Inventory
                auto slot = get_hovered_creative_slot();
                if (slot) {
                    m_selected = slot->stack();

                    if (m_selected && m_selected->item != 0) {
                        m_selected->count = 64;
                        auto it = item_textures.find(m_selected->item);
                        ASSERT(it != item_textures.end());
                        m_selected_image->set_texture(it->second.get(), false);
                        m_selected_image->set_visible(true);
                        return true;
                    }
                }
            } else {
                // Backpack Inventory
                auto [slot, pos] = get_hovered_backpack_slot();
                if (slot) {
                    m_selected = slot->stack();
                    if (m_selected && m_selected->item != 0) {
                        auto it = item_textures.find(m_selected->item);
                        ASSERT(it != item_textures.end());
                        m_selected_image->set_texture(it->second.get(), false);
                        m_from = pos;
                        m_selected_image->set_visible(true);
                        return true;
                    }
                }
            }
            {
                auto [slot, pos] = get_hovered_hotbar_slot();
                if (slot) {
                    m_selected = slot->stack();
                    if (m_selected && m_selected->item != 0) {
                        auto it = item_textures.find(m_selected->item);
                        ASSERT(it != item_textures.end());
                        m_selected_image->set_texture(it->second.get(), false);

                        m_from = pos;
                        m_selected_image->set_visible(true);
                        return true;
                    }
                }
            }
        }
        if (m_selected_image->has_texture()) {
            m_selected_image->set_texture(nullptr, false);
            m_selected_image->set_visible(false);
            auto handle_slot = [this](ItemSlot* slot, size_t pos) {
                if (slot && m_selected) {
                    auto& player = m_scene.client_world().get_player();
                    if (m_from) {
                        player.move_item(*m_from, pos);
                    } else {
                        player.add_item(pos, m_selected->item,
                                        m_selected->count);
                    }
                }
            };
            {
                auto [slot, pos] = get_hovered_hotbar_slot();
                handle_slot(slot, pos);
            }
            if (m_current == 1) {
                auto [slot, pos] = get_hovered_backpack_slot();
                handle_slot(slot, pos);
            }
            m_selected.reset();
            m_from.reset();
            return true;
        }
    }
    return UIManager::handle_mouse_button_event(e);
}
ItemSlot* InventoryUI::get_hovered_creative_slot() {

    for (auto& slot : m_creative_slots) {
        if (slot->hovered()) {

            return slot;
        }
    }
    return nullptr;
}

std::pair<ItemSlot*, size_t> InventoryUI::get_hovered_hotbar_slot() {
    for (size_t i = 0; i < HOTBAR_SIZE; ++i) {
        auto& slot = m_hotbar[i];
        if (slot->hovered()) {

            return {slot, i};
        }
    }
    return {nullptr, 0};
}

std::pair<ItemSlot*, size_t> InventoryUI::get_hovered_backpack_slot() {
    for (size_t i = 0; i < BACKPACK_SIZE; ++i) {
        auto& slot = m_backpack_slots[i];
        if (slot->hovered()) {
            return {slot, i + HOTBAR_SIZE};
        }
    }
    return {nullptr, 0};
}

} // namespace cubed
