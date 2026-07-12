#include "Cubed/tools/font.hpp"

#include "Cubed/constants.hpp"
#include "Cubed/tools/log.hpp"

namespace fs = std::filesystem;

namespace Cubed {

Font::Font() {

    if (FT_Init_FreeType(&m_ft)) {
        Logger::error("FREETYPE: Could not init FreeType Library");
    }
    if (FT_New_Face(m_ft, font_path().c_str(), 0, &m_face)) {
        Logger::error("FREETYPE: Failed to load font");
    }

    FT_Set_Pixel_Sizes(m_face, 0, 48);
    setup_font_character();
}

Font::~Font() {

    FT_Done_Face(m_face);
    FT_Done_FreeType(m_ft);
}

void Font::load_character(char8_t c) {
    if (FT_Load_Char(m_face, c, FT_LOAD_RENDER)) {
        Logger::error("FREETYTPE: Failed to load Glyph");
        return;
    }
    const auto& width = m_face->glyph->bitmap.width;
    const auto& height = m_face->glyph->bitmap.rows;
    m_text_texture->tex_sub_image_3d(TextureFormat::RED, GL_UNSIGNED_BYTE,
                                     m_face->glyph->bitmap.buffer, 0, 0,
                                     static_cast<int>(c), width, height);

    Character character = {
        glm::vec2{0.5f / m_texture_width, 0.5f / m_texture_height},
        glm::vec2{(width - 0.5f) / m_texture_width,
                  (height - 0.5f) / m_texture_height},
        glm::ivec2(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows),
        glm::ivec2(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top),
        static_cast<GLuint>(m_face->glyph->advance.x)};

    m_characters.insert({c, std::move(character)});
}

void Font::setup_font_character() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    m_text_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D_ARRAY);
    m_text_texture->tex_image_3d(TextureFormat::R8, TextureFormat::RED,
                                 GL_UNSIGNED_BYTE, nullptr, m_texture_width,
                                 m_texture_height, MAX_CHARACTER);

    for (char8_t c = 0; c < 128; c++) {
        load_character(c);
    }
    m_text_texture->set_linear();
    m_text_texture->set_clamp_to_edge(false, true, true);
}

std::vector<Vertex2D> Font::vertices(const std::string& text) {
    static Font font;

    std::vector<Vertex2D> vertices;

    float pen_x = 0.0f;
    float pen_y = 0.0f;

    for (char8_t c : text) {
        auto it = font.m_characters.find(c);
        if (it == font.m_characters.end()) {
            Logger::error("Can't find character {}", static_cast<char>(c));
            continue;
        }

        Character& ch = it->second;

        float xpos = pen_x + ch.bearing.x;
        float ypos = pen_y - ch.bearing.y;

        float w = ch.size.x;
        float h = ch.size.y;

        vertices.emplace_back(xpos, ypos + h, ch.uv_min.x, ch.uv_max.y,
                              static_cast<float>(c));
        vertices.emplace_back(xpos, ypos, ch.uv_min.x, ch.uv_min.y,
                              static_cast<float>(c));
        vertices.emplace_back(xpos + w, ypos, ch.uv_max.x, ch.uv_min.y,
                              static_cast<float>(c));

        vertices.emplace_back(xpos, ypos + h, ch.uv_min.x, ch.uv_max.y,
                              static_cast<float>(c));
        vertices.emplace_back(xpos + w, ypos, ch.uv_max.x, ch.uv_min.y,
                              static_cast<float>(c));
        vertices.emplace_back(xpos + w, ypos + h, ch.uv_max.x, ch.uv_max.y,
                              static_cast<float>(c));

        pen_x += (ch.advance >> 6);
    }

    return vertices;
}

const Texture* Font::text_texture() { return m_text_texture.get(); }

const std::string& Font::font_path() { return m_font_path; }

} // namespace Cubed