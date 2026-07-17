#pragma once
#include "Cubed/ui/text.hpp"
#include "Cubed/ui/ui_vertex_data.hpp"
#include "Cubed/ui/widget.hpp"
namespace Cubed {
class Label : public Widget {
public:
    Label(const Label&) = delete;
    Label(Label&&) = delete;
    Label& operator=(const Label&) = delete;
    Label& operator=(Label&&) = delete;

    Label(Widget* parent);
    virtual ~Label() = default;

    Label& set_text(std::string_view text);

    Label& set_color(Color color);
    Label& set_scale(float scale);

    const UIVertexData& data() const;

    const TextStyle& text_style() const;

    float width() const override;
    float height() const override;
    float real_width() const;
    float real_height() const;
    float offset_x() const;
    float offset_y() const;
    float scale() const;
    const std::string& text() const;

protected:
    virtual void on_update(float dt) override;
    virtual void on_render(Renderer& renderer) override;

private:
    TextStyle m_text;
    UIVertexData m_data;
    float m_real_width = 0.0f;
    float m_real_height = 0.0f;
    float m_offset_x = 0.0f;
    float m_offset_y = 0.0f;
    float m_scale = 1.0f;
    void update_vertices();
};
} // namespace Cubed