#include "Cubed/ui/label.hpp"

#include "Cubed/render/renderer.hpp"
#include "Cubed/tools/font.hpp"
namespace Cubed {
Label::Label(const std::string& id) : Widget(id) {}
Label::Label() {}
Label& Label::set_text(std::string_view text) {
    m_text.text = text;
    m_dirty = true;
    return *this;
}

Label& Label::set_color(Color color) {
    m_text.color = color;
    return *this;
}

void Label::update(float dt) { on_update(dt); }

void Label::on_update(float dt) {
    (void)dt;
    if (m_dirty) {
        update_vertices();
        m_dirty = false;
    }
}

void Label::render(Renderer& renderer) { on_render(renderer); }

void Label::on_render(Renderer& renderer) { renderer.render_lable(*this); }

void Label::update_vertices() {
    m_data.m_vertices = Font::vertices(m_text.text);
    m_data.update_sum();
    m_data.upload();
}

const UIVertexData& Label::data() const { return m_data; }
const TextStyle& Label::text_style() const { return m_text; }

} // namespace Cubed