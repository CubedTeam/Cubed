#pragma once

#include "Cubed/ui/widget.hpp"

#include <functional>

namespace Cubed {
class Button : public Widget {
public:
    Button();

    template <typename F> Button& set_clicked(F&& f) {
        m_clicked = std::move(f);
    }

private:
    std::function<void()> m_clicked;
};
} // namespace Cubed