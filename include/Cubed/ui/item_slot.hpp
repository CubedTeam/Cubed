#pragma once

#include "Cubed/gameplay/item.hpp"
#include "Cubed/gameplay/item_stack.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/label.hpp"
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
    ItemSlot& set_item(std::optional<ItemStack> stack, const Texture* texture);
    ItemSlot& set_enable(bool enable);

    float width() const override;
    float height() const override;
    bool handle_mouse_move_event(const MouseMoveEvent& e) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;
    std::optional<ItemID> id() const;
    std::optional<ItemStack> stack() const;
    bool hovered() const;

    template <typename F> ItemSlot& set_clicked(F&& f) {
        m_clicked = std::forward<F>(f);
        return *this;
    }

private:
    static constexpr const char* DEFAULT_SLOT_PATH =
        "cubed/textures/ui/slot.png";
    void on_render(Renderer& renderer) override;
    void on_update(float dt) override;
    std::function<void()> m_clicked;
    std::unique_ptr<Label> m_label;
    std::unique_ptr<Image> m_background;
    std::unique_ptr<Image> m_foreground;
    std::optional<ItemStack> m_item;
    float m_scale = 1.0f;
    bool m_hovered = false;
    bool m_enable = true;
};
} // namespace cubed
