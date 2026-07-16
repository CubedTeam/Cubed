#include "Cubed/ui/combo_button.hpp"

namespace Cubed {
ComboButton::ComboButton(Widget* parent) : Button(parent) {}

Button& ComboButton::set_text(const std::string& text) {
    m_text = text;
    update_text();
    return *this;
}
bool ComboButton::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {
        if (m_hovered && m_enable) {
            m_index = (m_index + 1) % m_sum;
            update_text();
            if (m_funs[m_index]) {
                m_funs[m_index]();
            } else {
                Logger::error("Function {} is null", m_index);
            }
            return true;
        }
    }

    return Widget::handle_mouse_button_event(e);
}
ComboButton& ComboButton::set_combos(
    std::span<std::pair<std::string, std::function<void()>>> combos) {
    m_funs.clear();
    m_suffix.clear();
    m_sum = combos.size();
    for (auto& pair : combos) {
        m_suffix.push_back(pair.first);
        m_funs.push_back(pair.second);
    }
    update_text();
    return *this;
}

ComboButton& ComboButton::set_index(int index) {
    m_index = index;
    update_text();
    return *this;
}

void ComboButton::update_text() {
    if (m_suffix.empty()) {
        return;
    }
    auto text = m_text + ": " + m_suffix[m_index];
    m_foreground->set_text(text);
    update_text_scale();
}

} // namespace Cubed