#pragma once
#include "Cubed/ui/text.hpp"
#include "Cubed/ui/ui_vertex_data.hpp"
#include "Cubed/ui/widget.hpp"
#include "glm/ext/vector_float2.hpp"
namespace Cubed {
class Label : public Widget {
public:
    Label(const Label&) = delete;
    Label(Label&&) = delete;
    Label& operator=(const Label&) = delete;
    Label& operator=(Label&&) = delete;

    Label(const std::string& id);
    virtual ~Label() = default;

    Label& set_position(const glm::vec2& pos);
    Label& set_position(float x, float y);
    Label& set_text(std::string_view text);
    Label& set_scale(float scale);
    Label& set_color(Color color);
    virtual void update(float dt) override;
    virtual void render(Renderer& renderer) override;

    const UIVertexData& data() const;
    const glm::vec2& pos() const;
    const TextStyle& text_style() const;
    float scale() const;

protected:
    virtual void on_update(float dt) override;
    virtual void on_render(Renderer& renderer) override;

private:
    float m_scale = 1.0f;
    glm::vec2 m_pos{0.0f, 0.0f};
    TextStyle m_text;
    UIVertexData m_data;
    bool m_dirty = false;
    void update_vertices();
};
} // namespace Cubed