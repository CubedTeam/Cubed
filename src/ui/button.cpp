#include "Cubed/ui/button.hpp"

#include <algorithm>

namespace Cubed {
Button::Button(Widget* parent) : Widget(parent) {
    m_background = std::make_unique<Image>(this);
    m_background->set_fill_parent(true);
    m_background->set_anchor(Anchor::TOP_LEFT);
    m_foreground = std::make_unique<Label>(this);
    m_foreground->set_anchor(Anchor::CENTER);
    set_width(NORMAL_BUTTON_WIDTH);
    set_height(NORMAL_BUTTON_HEIGHT);
}

void Button::on_update(float dt) {
    Widget::on_update(dt);
    if (m_background) {
        m_background->update(dt);
    }
    if (m_foreground) {
        m_foreground->update(dt);
    }
}

void Button::on_render(Renderer& renderer) {
    if (m_background) {
        m_background->render(renderer);
    }
    if (m_foreground) {
        m_foreground->render(renderer);
    }
    Widget::on_render(renderer);
}

bool Button::handle_mouse_move_event(const MouseMoveEvent& e) {
    if (m_enable) {
        auto p = pos();
        if (e.xpos >= p.x && e.xpos <= p.x + width() && e.ypos >= p.y &&
            e.ypos <= p.y + height()) {
            m_hovered = true;
            set_border_visale(true);
            return true;
        }
    }
    m_hovered = false;
    set_border_visale(false);

    return Widget::handle_mouse_move_event(e);
}
bool Button::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {
        if (m_hovered && m_clicked && m_enable) {
            m_clicked();

            return true;
        }
    }

    return Widget::handle_mouse_button_event(e);
}

Button& Button::set_scale(float scale) {
    m_scale = scale;
    update_text_scale();
    update_border();
    return *this;
}

float Button::scale() const { return m_scale; }

float Button::width() const {

    if (m_fill_width || m_fill_parent) {
        return Widget::width();
    }
    return Widget::width() * m_scale;
}
float Button::height() const {

    if (m_fill_width || m_fill_parent) {
        return Widget::height();
    }
    return Widget::height() * m_scale;
}
Button& Button::set_width(float width) {
    Widget::set_width(width);
    update_text_scale();
    return *this;
}
Button& Button::set_height(float height) {
    Widget::set_height(height);
    update_text_scale();
    return *this;
}

Button& Button::set_background_image(const std::string& path,
                                     TextureManager& texture_manager) {
    m_background->set_image(path, texture_manager, false);
    return *this;
}

Button& Button::set_default_image(TextureManager& texture_manager) {
    return set_background_image(DEFAULT_BUTTON_IMAGE, texture_manager);
}

Button& Button::set_text(const std::string& text) {
    m_foreground->set_text(text);
    update_text_scale();
    return *this;
}

Button& Button::set_auto_scale(bool auto_scale) {
    m_auto_scale = auto_scale;
    return *this;
}
Button& Button::set_enable(bool enable) {
    m_enable = enable;
    return *this;
}
void Button::update_text_scale() {
    if (!m_auto_scale) {
        m_foreground->set_scale(TEXT_SCALE);
        return;
    }
    float text_w = m_foreground->real_width();
    float text_h = m_foreground->real_height();

    if (text_w <= 0.0f || text_h <= 0.0f)
        return;

    float available_w = std::max(0.0f, width() - 2.0f * PADDING * m_scale);
    float available_h = std::max(0.0f, height() - 2.0f * PADDING * m_scale);

    float scale_x = available_w / text_w;
    float scale_y = available_h / text_h;

    float scale = std::max(0.0f, std::min(scale_x, scale_y));
    scale = std::clamp(scale, 0.01f, 100.0f);
    m_foreground->set_scale(scale);
}

} // namespace Cubed