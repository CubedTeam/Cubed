#include "Cubed/ui/label.hpp"

#include "Cubed/render/renderer.hpp"
#include "Cubed/tools/font.hpp"
namespace Cubed {
Label::Label(const std::string& id, Widget* parent) : Widget(id, parent) {}
Label::Label(Widget* parent) : Widget(parent) {}
Label& Label::set_text(std::string_view text) {
    m_text.text = text;
    update_vertices();
    return *this;
}

Label& Label::set_color(Color color) {
    m_text.color = color;
    return *this;
}
Label& Label::set_scale(float scale) {
    m_scale = scale;
    return *this;
}

void Label::on_update(float dt) { (void)dt; }

void Label::on_render(Renderer& renderer) { renderer.render_lable(*this); }

void Label::update_vertices() {
    auto textmesh = Font::vertices(m_text.text);
    m_data.m_vertices = std::move(textmesh.vertices);

    m_offset_x = textmesh.min_x;
    m_offset_y = textmesh.min_y;
    m_real_width = textmesh.width;
    m_real_height = textmesh.height;

    m_data.update_sum();
    m_data.upload();
}

const UIVertexData& Label::data() const { return m_data; }
const TextStyle& Label::text_style() const { return m_text; }
float Label::width() const { return m_real_width * m_scale; }
float Label::height() const { return m_real_height * m_scale; }
float Label::real_width() const { return m_real_width; }
float Label::real_height() const { return m_real_height; }
float Label::offset_x() const { return m_offset_x; }
float Label::offset_y() const { return m_offset_y; }
float Label::scale() const { return m_scale; }
const std::string& Label::text() const { return m_text.text; }
} // namespace Cubed