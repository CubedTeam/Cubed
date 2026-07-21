#pragma once

#include "Cubed/gameplay/block.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/widget.hpp"
namespace Cubed {
class TextureManager;
class ItemSlot : public Widget {
public:
    static constexpr float DEFAULT_WIDTH = 16.0f;
    static constexpr float DEFAULT_HEIGHT = 16.0f;
    ItemSlot(Widget* parent);

    ItemSlot& set_default_background(TextureManager& m_texture_manager);
    ItemSlot& set_scale(float m_scale);
    ItemSlot& set_item(BlockType id, const Texture* texture);
    float width() const override;
    float height() const override;

private:
    static constexpr const char* DEFAULT_SLOT_PATH = "texture/ui/slot.png";
    void on_render(Renderer& renderer) override;
    void on_update(float dt) override;
    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_window_resize_event(const WindowResizeEvent& e) override;
    std::unique_ptr<Image> m_background;
    std::unique_ptr<Image> m_foreground;
    std::unique_ptr<Label> m_label;
    BlockType m_block_type;
    float m_scale = 1.0f;
    bool m_hovered = false;
};
} // namespace Cubed