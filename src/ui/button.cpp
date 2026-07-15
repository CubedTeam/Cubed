#include "Cubed/ui/button.hpp"

namespace Cubed {
Button::Button(Widget* parent) : Widget(parent) {
    m_background = std::make_unique<Image>(this);
    m_background->set_fill(true);
    m_background->set_anchor(Anchor::TOP_LEFT);
    m_foreground = std::make_unique<Label>(this);
    m_foreground->set_anchor(Anchor::CENTER);
}

void Button::on_update(float dt) {
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
}

bool Button::handle_mouse_move_event(const MouseMoveEvent& e) {
    auto p = pos();
    if (e.xpos >= p.x && e.xpos <= p.x + width() && e.ypos >= p.y &&
        e.ypos <= p.y + height()) {
        m_hovered = true;
        return true;
    }
    m_hovered = false;

    return Widget::handle_mouse_move_event(e);
}
bool Button::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {
        if (m_hovered && m_clicked) {
            m_clicked();

            return true;
        }
    }

    return Widget::handle_mouse_button_event(e);
}

Button& Button::set_scale(float scale) {
    m_scale = scale;
    update_text_scale();
    return *this;
}

float Button::scale() const { return m_scale; }

float Button::width() const { return m_width * m_scale; }
float Button::height() const { return m_height * m_scale; }
Button& Button::set_width(float width) {
    m_width = width;
    update_text_scale();
    return *this;
}
Button& Button::set_height(float height) {
    m_height = height;
    update_text_scale();
    return *this;
}

Button& Button::set_background_image(const std::string& path,
                                     TextureManager& texture_manager) {
    m_background->set_image(path, texture_manager);
    return *this;
}
Button& Button::set_text(const std::string& text) {
    m_foreground->set_text(text);
    update_text_scale();
    return *this;
}

void Button::update_text_scale() {
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