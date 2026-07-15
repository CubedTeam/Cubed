#include "Cubed/ui/text_field.hpp"

#include "Cubed/tools/text_tools.hpp"

namespace Cubed {
TextField::TextField(Widget* parent) : Widget(parent) {

    m_background = std::make_unique<Image>(this);
    m_background->set_fill(true);
    m_background->set_anchor(Anchor::TOP_LEFT);

    m_foreground = std::make_unique<Label>(this);
    m_foreground->set_anchor(Anchor::CENTER_LEFT);
    m_foreground->set_offset({10, 0});
    update_text_scale();
}

void TextField::update_text_scale() {
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
void TextField::on_render(Renderer& renderer) {
    if (m_background) {
        m_background->render(renderer);
    }
    if (m_foreground) {
        m_foreground->render(renderer);
    }
}
void TextField::on_update(float dt) {
    if (m_background) {
        m_background->update(dt);
    }
    if (m_foreground) {
        m_foreground->update(dt);
    }
}

void TextField::update_show_text() {
    if (m_input_text.empty()) {
        if (!m_typing) {
            m_foreground->set_text(m_show_text);
            m_foreground->set_color(Color::GRAY);
        } else {
            m_foreground->set_text(" ");
        }

    } else {
        m_foreground->set_text(m_input_text);
        m_foreground->set_color(Color::WHITE);
    }
    update_text_scale();
}

TextField& TextField::set_scale(float scale) {
    m_scale = scale;
    return *this;
}

float TextField::width() const { return m_width * m_scale; }
float TextField::height() const { return m_height * m_scale; }

TextField& TextField::set_width(float width) {
    m_width = width;
    update_text_scale();
    return *this;
}
TextField& TextField::set_height(float height) {
    m_height = height;
    update_text_scale();
    return *this;
}
TextField& TextField::set_show_text(const std::string& text) {
    m_show_text = text;
    update_show_text();
    return *this;
}
TextField& TextField::set_background_image(const std::string& path,
                                           TextureManager& texture_manager) {
    m_background->set_image(path, texture_manager);
    return *this;
}

TextField& TextField::set_default_image(TextureManager& texture_manager) {
    return set_background_image(DEFAULT_TEXT_FIELD_IMAGE, texture_manager);
}

TextField& TextField::set_auto_scale(bool auto_scale) {
    m_auto_scale = auto_scale;
    update_text_scale();
    return *this;
}

const std::string& TextField::input_text() const { return m_input_text; }

bool TextField::handle_mouse_move_event(const MouseMoveEvent& e) {
    auto p = pos();
    if (e.xpos >= p.x && e.xpos <= p.x + width() && e.ypos >= p.y &&
        e.ypos <= p.y + height()) {
        m_inside = true;
    } else {
        m_inside = false;
    }

    return false;
}
bool TextField::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.key == MouseKey::LEFT_BUTTON && e.action == KeyAction::PRESS) {
        if (m_inside) {
            m_typing = true;
            update_show_text();
            return true;
        } else {
            if (m_typing) {
                m_typing = false;
                if (m_on_finished) {
                    m_on_finished();
                }
                return true;
            }
        }
    }
    return false;
}
bool TextField::handle_text_input_event(const TextInputEvent& e) {
    if (m_typing) {
        m_input_text.append(e.text);
        update_show_text();
        return true;
    }
    return false;
}
bool TextField::handle_key_event(const KeyEvent& e) {
    if (e.key == Key::ENTER && e.action == KeyAction::PRESS) {
        if (m_typing) {
            m_typing = false;
            if (m_on_finished) {
                m_on_finished();
            }
            return true;
        }
    }
    if (e.key == Key::BACKSPACE &&
        (e.action == KeyAction::PRESS || e.action == KeyAction::REPEAT)) {
        if (m_typing) {
            utf8_pop_back(m_input_text);
            update_show_text();
            return true;
        }
    }
    return false;
}
} // namespace Cubed