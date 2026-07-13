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

    Label(const std::string& id);
    Label();
    virtual ~Label() = default;

    Label& set_text(std::string_view text);

    Label& set_color(Color color);
    virtual void update(float dt) override;
    virtual void render(Renderer& renderer) override;

    const UIVertexData& data() const;

    const TextStyle& text_style() const;

protected:
    virtual void on_update(float dt) override;
    virtual void on_render(Renderer& renderer) override;

private:
    TextStyle m_text;
    UIVertexData m_data;
    bool m_dirty = false;
    void update_vertices();
};
} // namespace Cubed