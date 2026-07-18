#include "Cubed/ui/chat_box.hpp"

#include "Cubed/tools/text_tools.hpp"
namespace Cubed {
ChatBox::ChatBox(Widget* parent) : Widget(parent) {

    m_text_field = std::make_unique<TextField>(this);
}

void ChatBox::add_message(ChatMessage& message) {
    while (m_messages.size() > MAX_MESSGAES_SUM) {
        m_messages.pop_front();
    }

    std::string str = std::format("<{}>{}", message.player, message.text);

    auto split_str = split_utf8(str, 20);
    for (auto it = split_str.rbegin(); it != split_str.rend(); ++it) {
        auto lable = std::make_unique<Label>(this);
        lable->set_text(*it)
            .set_scale(m_text_scale)
            .set_anchor(Anchor::TOP_LEFT);
        m_messages.emplace_back(std::move(lable), message.time);
    }
}

ChatBox& ChatBox::set_scale(float scale) {
    m_scale = scale;
    m_text_field->set_scale(scale);
    return *this;
}
ChatBox& ChatBox::set_text_scale(float scale) {
    m_text_scale = scale;

    for (auto& text : m_messages) {
        text.label->set_scale(scale);
    }

    return *this;
}
ChatBox& ChatBox::set_width(float width) {
    m_width = width;
    m_text_field->set_width(width);
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
float ChatBox::width() const { return m_width * m_scale; }
float ChatBox::height() const { return m_height * m_scale; }
void ChatBox::set_d_image(TextureManager& m) {
    m_text_field->set_default_image(m);
}

void ChatBox::set_typing(bool typing) { m_text_field->set_typing(typing); }
void ChatBox::set_app(App* app) { m_text_field->set_app(app); }

void ChatBox::layout() {
    set_offset({0, m_text_field->height()});
    int y = 0;
    int line = 0;
    for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it) {
        if (!it->label) {
            continue;
        }
        if (line > m_lines) {
            it->render = false;
            continue;
        }
        ++line;
        it->label->set_anchor(Anchor::TOP_LEFT);
        it->label->set_offset({0, y});
        it->render = true;
        y += it->label->height() + m_spacing;
    }
    m_height = m_lines == 0 ? 0 : (y - m_spacing);
}
void ChatBox::on_update(float dt) {
    layout();

    m_text_field->update(dt);
}

void ChatBox::on_render(Renderer& renderer) {
    m_text_field->render(renderer);
    for (auto it = m_messages.rbegin(); it != m_messages.rend(); ++it) {
        if (!it->label) {
            continue;
        }
        if (it->render) {
            it->label->render(renderer);
        }
    }
}
bool ChatBox::handle_text_input_event(const TextInputEvent& e) {
    if (m_text_field->handle_text_input_event(e)) {
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

} // namespace Cubed