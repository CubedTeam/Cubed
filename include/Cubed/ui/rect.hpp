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

    void update(float dt) override;
    void render(Renderer& renderer) override;

    float width() const override;
    float height() const override;
    float alpha() const;

    Rect& set_width(float width);
    Rect& set_height(float height);

    Rect& set_fill(bool fill);

    Rect& set_scale(float scale);
    Rect& set_color(Color color);
    Rect& set_alpha(float alpha);

    Color color() const;

private:
    Color m_color = Color::WHITE;
    float m_width = 0.0f;
    float m_height = 0.0f;
    float m_scale = 1.0f;
    float m_alpha = 1.0f;
    bool m_fill = false;
};
} // namespace Cubed