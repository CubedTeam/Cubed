#include "Cubed/ui/chat_box.hpp"

#include "Cubed/tools/font.hpp"
#include "Cubed/tools/time_tools.hpp"

#include <utf8cpp/utf8.h>
namespace Cubed {
ChatBox::ChatBox(Widget* parent) : Widget(parent) {}

void ChatBox::add_message(ChatMessage& message) {
    while (m_messages.size() > MAX_MESSGAES_SUM) {
        m_messages.pop_front();
    }

    std::string str = message.system_msg
                          ? std::format("{}", message.text)
                          : std::format("<{}>{}", message.player, message.text);

    auto split_str = wrap_message(str);
    for (auto it = split_str.begin(); it != split_str.end(); ++it) {
        auto lable = std::make_unique<Label>(this);
        lable->set_text(*it).set_scale(m_text_scale).set_color(message.color);
        auto background = std::make_unique<Rect>(lable.get());
        background->set_fill_height(true);
        background->set_anchor(Anchor::TOP_LEFT);
        background->set_alpha(0.6f)
            .set_color(Color::GRAY)
            .set_width(text_label_width());
        lable->set_background(std::move(background))
            .set_anchor(Anchor::TOP_LEFT);
        m_messages.emplace_back(std::move(lable), message.time);
    }
}

ChatBox& ChatBox::set_scale(float scale) {
    m_scale = scale;
    if (m_text_field) {
        m_text_field->set_scale(scale);
    }

    return *this;
}
ChatBox& ChatBox::set_text_scale(float scale) {
    m_text_scale = scale;

    for (auto& text : m_messages) {
        text.label->set_scale(scale);
    }

    return *this;
}
ChatBox& ChatBox::set_show_lines(int lines) {
    m_lines = lines;
    return *this;
}
ChatBox& ChatBox::set_spacing(float spacing) {
    m_spacing = spacing;
    return *this;
}
ChatBox& ChatBox::clear_input() {
    m_text_field->clear_input();
    return *this;
}
std::string& ChatBox::get_input_text() { return m_text_field->input_text(); }

float ChatBox::width() const {
    if (m_fill_width || m_fill_parent) {
        return Widget::width();
    }
    return Widget::width() * m_scale;
}
float ChatBox::height() const {
    // Height is dynamically calculated, no scaling needed
    return Widget::height();
}
float ChatBox::text_label_width() const { return m_text_width * m_scale; }
void ChatBox::set_text_field(std::unique_ptr<TextField> text_field) {
    m_text_field = std::move(text_field);
}

void ChatBox::set_typing(bool typing, bool finished) {
    if (m_text_field) {
        m_text_field->set_typing(typing, finished);
        m_text_field->set_visible(typing);
    }
}

void ChatBox::layout() {
    int y;
    if (m_text_field) {
        y = m_text_field->height() + m_spacing;
    } else {
        y = m_spacing;
    }
    int line = 0;
    // Shift upward sequentially starting from the latest
    for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it) {
        if (!it->label) {
            continue;
        }
        if (line > m_lines) {
            it->render = false;
            continue;
        }
        ++line;
        it->label->set_anchor(Anchor::BOTTOM_LEFT);
        // -y means upward offset
        it->label->set_offset({0, -y});
        y += it->label->height() + m_spacing;
    }
    set_height(m_lines == 0 ? 0 : (y - m_spacing));
}
void ChatBox::on_update(float dt) {
    Widget::on_update(dt);
    auto tick = Tools::get_time_ticks();
    for (auto& m : m_messages) {
        m.label->update(dt);
        if (!m.render) {
            continue;
        }

        if (tick < m.time) {
            Logger::error("Tick {} Less Than Line Time {}", tick, m.time);
            continue;
        }
        auto elapsed = tick - m.time;

        if (elapsed >= DISAPPEAR_TIME * 1000) {

            m.render = false;
        }
    }
    layout();
    m_text_field->update(dt);
}

void ChatBox::on_render(Renderer& renderer) {
    if (m_text_field) {
        m_text_field->render(renderer);
    }

    for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it) {
        if (!it->label) {
            continue;
        }
        if (m_text_field->is_typing() || it->render) {
            it->label->render(renderer);
        }
    }
    Widget::on_render(renderer);
}
bool ChatBox::handle_text_input_event(const TextInputEvent& e) {
    if (m_text_field && m_text_field->handle_text_input_event(e)) {
        return true;
    }
    return Widget::handle_text_input_event(e);
}

bool ChatBox::handle_key_event(const KeyEvent& e) {
    if (m_text_field->handle_key_event(e)) {
        return true;
    }
    return Widget::handle_key_event(e);
}
std::vector<std::string> ChatBox::wrap_message(const std::string& text) const {
    std::vector<std::string> lines;

    std::string line;
    size_t last_space = std::string::npos;

    auto it = text.begin();
    while (it != text.end()) {
        auto begin = it;
        utf8::next(it, text.end());

        line.append(begin, it);

        if (*begin == ' ')
            last_space = line.size() - 1;

        if (Font::text_width(line) * m_text_scale > text_label_width()) {
            if (last_space != std::string::npos) {
                lines.emplace_back(line.substr(0, last_space));
                line.erase(0, last_space + 1);
                last_space = std::string::npos;
            } else {

                line.erase(line.size() - (it - begin));
                lines.emplace_back(line);
                line.assign(begin, it);
            }
        }
    }

    if (!line.empty())
        lines.emplace_back(std::move(line));

    return lines;
}
} // namespace Cubed