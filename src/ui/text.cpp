#include "Cubed/ui/text.hpp"

#include "Cubed/shader.hpp"
#include "Cubed/tools/cubed_hash.hpp"
#include "Cubed/tools/font.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Cubed {

Text::Text(std::string_view name)
    : NAME(name), UUID(HASH::str(name)),
      m_vbo(std::make_unique<VertexBuffer>()),
      m_vao(std::make_unique<VertexArray>()) {}

Text::Text(std::string_view name, std::string_view str, glm::vec2 pos,
           Color color)
    : NAME(name), UUID(HASH::str(name)),
      m_vbo(std::make_unique<VertexBuffer>()),
      m_vao(std::make_unique<VertexArray>()) {
    m_text.assign(str);
    m_pos = pos;
    m_color = color_value(color);
    update_vertices();
}

Text::~Text() { m_vbo.reset(); }

Text::Text(Text&& other) noexcept
    : m_scale(other.m_scale), m_pos(other.m_pos), NAME(other.NAME),
      UUID(other.UUID), m_text(std::move(other.m_text)), m_color(other.m_color),
      m_model_matrix(other.m_model_matrix),
      m_vertices(std::move(other.m_vertices)), m_vbo(std::move(other.m_vbo)),
      m_vao(std::move(other.m_vao)) {}

Text& Text::color(Color color) {
    m_color = color_value(color);
    return *this;
}

Text& Text::position(float x, float y) {
    m_pos = glm::vec2{x, y};
    return *this;
}

Text& Text::scale(float s) {
    m_scale = s;
    return *this;
}

std::size_t Text::uuid() const { return UUID; }

Text& Text::text(std::string_view str) {
    m_text.assign(str);
    update_vertices();
    return *this;
}

void Text::render(const Shader& shader) {
    ASSERT_MSG(m_vbo != 0, "VBO not initialized!");
    ASSERT_MSG(!m_vertices.empty(), "Text String Not Set");
    Font::text_texture()->bind(0);
    m_vao->bind();
    m_model_matrix =
        glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x, m_pos.y, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(m_scale, m_scale, 1.0f));

    shader.set_loc("textColor", glm::vec3(m_color.x, m_color.y, m_color.z));
    shader.set_loc("mv_matrix", m_model_matrix);
    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
}

void Text::update_vertices() {
    m_vertices = Font::vertices(m_text);
    upload_to_gpu();
}

void Text::upload_to_gpu() {
    ASSERT_MSG(m_vbo, "Vbo Is Not Gen");
    m_vao->bind();
    m_vbo->buffer_data(m_vertices.data(), m_vertices.size() * sizeof(Vertex2D),
                       BufferUsage::DYNAMIC_DRAW);
    m_vao->attribute(0, 2, GL_FLOAT, sizeof(Vertex2D), (void*)0);
    m_vao->attribute(1, 2, GL_FLOAT, sizeof(Vertex2D),
                     (void*)offsetof(Vertex2D, s));
    m_vao->attribute(2, 1, GL_FLOAT, sizeof(Vertex2D),
                     (void*)offsetof(Vertex2D, layer));
}

bool Text::operator==(const Text& other) const { return UUID == other.uuid(); }

} // namespace Cubed