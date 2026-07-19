#pragma once

#include "Cubed/ui/color.hpp"
#include "Cubed/ui/widget.hpp"
namespace Cubed {
class Rect : public Widget {
public:
    Rect(const Rect&) = delete;
    Rect(Rect&&) = delete;
    Rect& operator=(const Rect&) = delete;
    Rect& operator=(Rect&&) = delete;
    Rect(Widget* parent);
    ~Rect();

    float width() const override;
    float height() const override;
    float alpha() const;

    Rect& set_scale(float scale);
    Rect& set_color(Color color);
    Rect& set_alpha(float alpha);

    Color color() const;

private:
    void on_update(float dt) override;
    void on_render(Renderer& renderer) override;

    Color m_color = Color::WHITE;
    float m_scale = 1.0f;
    float m_alpha = 1.0f;
};
} // namespace Cubed