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
        for (size_t i = 0; i < sum; ++i) {
            if (i % 10 == 0) {
                auto& r = column.add_child<RowLayout>();
                row = &r;
            }
            auto& slot = row->add_child<ItemSlot>();
            slot.set_default_background(texture_manager);
            slot.set_scale(5.0f);
            slot.set_item(i, block_textures[i].get());
        }
    }

    m_root_widget = std::move(back);
}
void InventoryUI::on_re_enter() {}
} // namespace Cubed