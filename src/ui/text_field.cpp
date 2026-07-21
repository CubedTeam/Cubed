#include "Cubed/ui/text_field.hpp"

#include "Cubed/app.hpp"
#include "Cubed/tools/text_tools.hpp"

#include <algorithm>

namespace {
constexpr float DELTA_CUROUR_HEIGHT = 10.0f;
}

namespace Cubed {
TextField::TextField(Widget* parent) : Widget(parent) {
    m_width = NORMAL_TEXTFIELD_WIDTH;
    m_height = NORMAL_TEXTFIELD_HEIGHT;
    m_foreground = std::make_unique<Label>(this);
    m_foreground->set_anchor(Anchor::CENTER_LEFT);
    m_foreground->set_offset({10, 0});

    m_cursor = std::make_unique<Rect>(this);
    m_cursor->set_width(3.0f);
    m_cursor->set_height(std::max(1.0f, height() - DELTA_CUROUR_HEIGHT));
    m_cursor->set_anchor(Anchor::CENTER_LEFT);
    m_cursor->set_offset({10, 0});
    m_cursor->set_color(Color::WHITE);

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
    if (m_cursor_visible && m_cursor && m_typing) {
        m_cursor->render(renderer);
    }
}
void TextField::on_update(float dt) {

    Widget::on_update(dt);

    m_cursor_timer += dt;
    if (m_cursor_timer >= CURSOR_INTERVAL) {
        m_cursor_timer = 0.0f;
        m_cursor_visible = !m_cursor_visible;
    }
    if (m_background) {
        m_background->update(dt);
    }
    if (m_foreground) {
        m_foreground->update(dt);
    }
    if (m_cursor) {
        m_cursor->update(dt);
    }
}

void TextField::update_show_text() {

    if (!m_typing && m_input_text.empty()) {
        m_foreground->set_text(m_show_text);
        m_foreground->set_color(Color::GRAY);
    } else {
        m_foreground->set_text(m_input_text);
        m_foreground->set_color(Color::WHITE);
    }

    update_text_scale();
    update_input_area();
    m_cursor->set_offset({13.0f + m_foreground->width(), 0.0f});
}

void TextField::update_input_area() {
    if (!m_app) {
        return;
    }
    auto p = pos();
    glm::vec4 textbox{p.x, p.y, width(), height()};
    float cursor_position_x = p.x + 13.0f + m_foreground->width();
    m_app->update_text_input_area(textbox, cursor_position_x);
}

TextField& TextField::set_scale(float scale) {
    m_scale = scale;
    m_cursor->set_height(std::max(1.0f, height() - DELTA_CUROUR_HEIGHT));

    return *this;
}

float TextField::width() const {

    if (m_fill_width) {
        return m_width;
    }
    return m_width * m_scale;
}
float TextField::height() const {
    if (m_fill_height) {
        return m_height;
    }
    return m_height * m_scale;
}

TextField& TextField::set_width(float width) {
    m_width = width;
    update_text_scale();
    return *this;
}
TextField& TextField::set_height(float h) {
    m_height = h;
    update_text_scale();
    m_cursor->set_height(std::max(1.0f, height() - DELTA_CUROUR_HEIGHT));
    return *this;
}
TextField& TextField::set_show_text(const std::string& text) {
    m_show_text = text;
    update_show_text();
    return *this;
}

TextField& TextField::set_background(std::unique_ptr<Widget> background) {
    m_background = std::move(background);
    return *this;
}

TextField& TextField::set_auto_scale(bool auto_scale) {
    m_auto_scale = auto_scale;
    update_text_scale();
    return *this;
}
TextField& TextField::set_app(App* app) {
    m_app = app;
    return *this;
}

TextField& TextField::set_typing(bool typing, bool finished) {
    m_typing = typing;
    if (m_typing) {
        start_typing();
    } else {
        stop_typing(finished);
    }

    return *this;
}

TextField& TextField::clear_input() {
    m_input_text.clear();
    update_show_text();
    return *this;
}
bool TextField::is_typing() const { return m_typing; }
void TextField::start_typing() {
    if (m_app) {
        m_app->start_text_input();
    }
    update_show_text();
}
void TextField::stop_typing(bool finished) {
    if (m_app) {
        m_app->stop_text_input();
    }
    if (m_on_finished && finished) {
        m_on_finished();
    }
}

const std::string& TextField::input_text() const { return m_input_text; }
std::string& TextField::input_text() { return m_input_text; }
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
            start_typing();
            return false;
        } else {
            if (m_typing) {
                m_typing = false;
                stop_typing(true);

                return false;
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
            stop_typing(true);

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
    if (e.key == Key::LEFT_CTRL) {
        if (m_typing) {
            if (e.action == KeyAction::PRESS) {
                m_ctrl_press = true;
                return true;
            } else if (e.action == KeyAction::RELEASE) {
                m_ctrl_press = false;
                return true;
            }
        }
    }

    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
        stop_typing(false);
        clear_input();
        return true;
    }

    if (e.key == Key::V && e.action == KeyAction::PRESS && m_ctrl_press) {
        if (m_app && m_typing) {
            m_input_text.append(m_app->get_clipboard_text());
            update_show_text();
            return true;
        }
    }

    return false;
}

bool TextField::handle_window_resize_event(const WindowResizeEvent& e) {
    Widget::handle_window_resize_event(e);
    update_input_area();
    return false;
}

} // namespace Cubed