#pragma once

#include "Cubed/gameplay/item.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/widget.hpp"
namespace cubed {
class TextureManager;
class ItemSlot : public Widget {
public:
    static constexpr float DEFAULT_WIDTH = 16.0f;
    static constexpr float DEFAULT_HEIGHT = 16.0f;
    ItemSlot(Widget* parent);

    ItemSlot& set_default_background(TextureManager& m_texture_manager);
    ItemSlot& set_scale(float m_scale);
    ItemSlot& set_item(ItemID id, const Texture* texture);
    float width() const override;
    float height() const override;
    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    ItemID id() const;
    bool hovered() const;

private:
    static constexpr const char* DEFAULT_SLOT_PATH =
        "cubed/textures/ui/slot.png";
    void on_render(Renderer& renderer) override;
    void on_update(float dt) override;
    std::unique_ptr<Image> m_background;
    std::unique_ptr<Image> m_foreground;
    ItemID m_id = 0;
    float m_scale = 1.0f;
    bool m_hovered = false;
};
} // namespace cubed
