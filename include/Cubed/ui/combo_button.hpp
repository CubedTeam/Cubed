#pragma once
#include "Cubed/ui/button.hpp"

#include <span>
namespace Cubed {
using ComboPair = std::pair<std::string, std::function<void()>>;
class ComboButton : public Button {

public:
    ComboButton(const ComboButton&) = delete;
    ComboButton(ComboButton&&) = delete;
    ComboButton& operator=(const ComboButton&) = delete;
    ComboButton& operator=(ComboButton&&) = delete;

    ComboButton(Widget* parent);
    Button& set_text(const std::string& text) override;
    bool handle_mouse_button_event(const MouseButtonEvent& e) override;
    ComboButton& set_combos(std::span<ComboPair> combos);
    ComboButton& set_index(int index);

private:
    void update_text();

    std::vector<std::string> m_suffix;
    std::vector<std::function<void()>> m_funs;
    std::string m_text;
    int m_index = 0;
    int m_sum = 0;
};
} // namespace Cubed