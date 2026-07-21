#include "Cubed/ui/item_slot.hpp"

namespace Cubed {
ItemSlot::ItemSlot(Widget* parent) : Widget(parent) {
    m_background = std::make_unique<Image>(this);
    m_background->set_fill_parent(true);
    m_foreground = std::make_unique<Image>(this);
    m_foreground->set_texture(nullptr, false).set_fill_parent(true);
    Widget::set_width(DEFAULT_WIDTH);
    Widget::set_height(DEFAULT_HEIGHT);
}

ItemSlot& ItemSlot::set_default_background(TextureManager& texture_manager) {
    m_background->set_image(DEFAULT_SLOT_PATH, texture_manager, false);
    return *this;
}
ItemSlot& ItemSlot::set_scale(float scale) {
    m_scale = scale;
    update_border();
    return *this;
}
ItemSlot& ItemSlot::set_item(BlockType id, const Texture* texture) {
    m_foreground->set_texture(texture, false);
    m_block_type = id;
    return *this;
}
float ItemSlot::width() const {
    if (m_fill_width || m_fill_parent) {
        return Widget::width();
    }
    return Widget::width() * m_scale;
}
float ItemSlot::height() const {
    if (m_fill_width || m_fill_parent) {
        return Widget::height();
    }
    return Widget::height() * m_scale;
}

void ItemSlot::on_render(Renderer& renderer) {
    Widget::on_render(renderer);
    if (m_background) {
        m_background->render(renderer);
    }
    if (m_foreground) {
        m_foreground->render(renderer);
    }
}
void ItemSlot::on_update(float dt) {

    if (m_background) {
        m_background->update(dt);
    }
    if (m_foreground) {
        m_foreground->update(dt);
    }
    Widget::on_update(dt);
}

bool ItemSlot::handle_mouse_move_event(const MouseMoveEvent& e) {
    auto p = pos();
    if (e.xpos >= p.x && e.xpos <= p.x + width() && e.ypos >= p.y &&
        e.ypos <= p.y + height()) {
        m_hovered = true;

    } else {
        m_hovered = false;
    }

    if (m_foreground && m_foreground->handle_mouse_move_event(e)) {
        return true;
    }
    if (m_background && m_background->handle_mouse_move_event(e)) {
        return true;
    }

    return Widget::handle_mouse_move_event(e);
}

BlockType ItemSlot::block() const { return m_block_type; }
bool ItemSlot::hovered() const { return m_hovered; }
} // namespace Cubed