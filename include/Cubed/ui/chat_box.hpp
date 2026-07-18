#pragma once

#include "Cubed/gameplay/chat_message.hpp"
#include "Cubed/ui/text_field.hpp"
#include "Cubed/ui/widget.hpp"

#include <deque>

namespace Cubed {
class ChatBox : public Widget {
public:
    ChatBox(Widget* parent);

    void add_message(ChatMessage& message);

    ChatBox& set_scale(float scale);
    ChatBox& set_text_scale(float scale);
    ChatBox& set_width(float width);
    ChatBox& set_show_lines(int lines);
    ChatBox& set_spacing(float spacing);
    float width() const override;
    float height() const override;
    void set_d_image(TextureManager& m);
    void set_typing(bool typing);
    void set_app(App* app);
    template <typename F> ChatBox& set_on_finish(F&& f) {
        m_text_field->set_on_finish(std::forward<F>(f));
        return *this;
    }

private:
    static constexpr int MAX_MESSGAES_SUM = 50;

    struct Line {
        std::unique_ptr<Label> label;
        uint64_t time;
        bool render = false;
    };
    int m_lines = 10;
    bool m_scale = 1.0f;
    float m_spacing = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;
    float m_text_scale = 1.0f;
    std::deque<Line> m_messages;
    std::unique_ptr<TextField> m_text_field;
    void layout();
    void on_update(float dt) override;
    void on_render(Renderer& renderer) override;
    bool handle_text_input_event(const TextInputEvent& e) override;
    bool handle_key_event(const KeyEvent& e) override;
};
} // namespace Cubed