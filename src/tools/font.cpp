#include "Cubed/tools/font.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <memory>

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

    GLint max_layers;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);

    m_max_layers = std::min(max_layers, 2048);
    Logger::info("Font Max Layers {}", m_max_layers);
    m_hb_font = hb_ft_font_create_referenced(m_face);
    m_text_texture = std::make_unique<Texture>(TextureType::TEXTURE_2D_ARRAY);
    m_text_texture->tex_image_3d(TextureFormat::R8, TextureFormat::RED,
                                 GL_UNSIGNED_BYTE, nullptr, CELL_SIZE,
                                 CELL_SIZE, m_max_layers);
    m_text_texture->set_linear();
    m_text_texture->set_clamp_to_edge();
}

Font::~Font() {
    hb_font_destroy(m_hb_font);
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_ft);
}

Glyph& Font::load_glyph(uint32_t glyph_index) {
    auto it = m_cache.find(glyph_index);
    if (it != m_cache.end()) {
        return it->second;
    }
    if (FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER)) {
        Logger::error("Failed to load glyph {}", glyph_index);
    }

    Glyph glyph;

    glyph.size.x = m_face->glyph->bitmap.width;
    glyph.size.y = m_face->glyph->bitmap.rows;
    glyph.bearing.x = m_face->glyph->bitmap_left;
    glyph.bearing.y = m_face->glyph->bitmap_top;

    upload_glyph(glyph, m_face->glyph->bitmap.buffer);
    auto [iter, _] = m_cache.try_emplace(glyph_index, std::move(glyph));
    return iter->second;
}

void Font::upload_glyph(Glyph& glyph, const unsigned char* buffer) {
    ASSERT(glyph.size.x <= CELL_SIZE);
    ASSERT(glyph.size.y <= CELL_SIZE);
    ASSERT(m_next_layer < m_max_layers);

    m_text_texture->tex_sub_image_3d(RED, GL_UNSIGNED_BYTE, buffer, 0, 0,
                                     m_next_layer, glyph.size.x, glyph.size.y);

    glyph.layer = m_next_layer++;

    glyph.uv_min = {0, 0};
    glyph.uv_max = {glyph.size.x / float(CELL_SIZE),
                    glyph.size.y / float(CELL_SIZE)};
}

Font& Font::get() { return *get_ptr(); }
std::unique_ptr<Font>& Font::get_ptr() {
    static std::unique_ptr<Font> font = std::make_unique<Font>();
    return font;
}
void Font::destroy() { get_ptr().reset(); }

TextMesh Font::vertices(const std::string& text) {

    hb_buffer_t* buffer = hb_buffer_create();
    hb_buffer_add_utf8(buffer, text.c_str(), text.size(), 0, text.size());
    hb_buffer_guess_segment_properties(buffer);
    hb_shape(m_hb_font, buffer, nullptr, 0);

    unsigned int count;

    hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &count);

    hb_glyph_position_t* positions =
        hb_buffer_get_glyph_positions(buffer, &count);
    if (count == 0) {
        hb_buffer_destroy(buffer);
        return {};
    }
    std::vector<Vertex2D> vertices;

    float min_x = std::numeric_limits<float>::max();
    float min_y = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float max_y = std::numeric_limits<float>::lowest();

    float pen_x = 0.0f;
    float pen_y = 0.0f;

    for (unsigned i = 0; i < count; i++) {

        uint32_t glyph_index = infos[i].codepoint;

        Glyph& glyph = load_glyph(glyph_index);

        float xpos = pen_x + positions[i].x_offset / 64.0f + glyph.bearing.x;

        float ypos = pen_y - positions[i].y_offset / 64.0f - glyph.bearing.y;

        float w = glyph.size.x;
        float h = glyph.size.y;

        min_x = std::min(min_x, xpos);
        min_y = std::min(min_y, ypos);

        max_x = std::max(max_x, xpos + w);
        max_y = std::max(max_y, ypos + h);

        vertices.emplace_back(xpos, ypos + h, glyph.uv_min.x, glyph.uv_max.y,
                              static_cast<float>(glyph.layer));
        vertices.emplace_back(xpos, ypos, glyph.uv_min.x, glyph.uv_min.y,
                              static_cast<float>(glyph.layer));
        vertices.emplace_back(xpos + w, ypos, glyph.uv_max.x, glyph.uv_min.y,
                              static_cast<float>(glyph.layer));

        vertices.emplace_back(xpos, ypos + h, glyph.uv_min.x, glyph.uv_max.y,
                              static_cast<float>(glyph.layer));
        vertices.emplace_back(xpos + w, ypos, glyph.uv_max.x, glyph.uv_min.y,
                              static_cast<float>(glyph.layer));
        vertices.emplace_back(xpos + w, ypos + h, glyph.uv_max.x,
                              glyph.uv_max.y, static_cast<float>(glyph.layer));

        pen_x += positions[i].x_advance / 64.0f;
        pen_y += positions[i].y_advance / 64.0f;
    }
    // Top-left anchor point
    for (auto& v : vertices) {
        v.x -= min_x;
        v.y -= min_y;
    }
    hb_buffer_destroy(buffer);
    return {std::move(vertices), max_x - min_x, max_y - min_y, 0, 0};
}

float Font::text_width(const std::string& text) {
    auto& f = Font::get();

    hb_buffer_t* buffer = hb_buffer_create();

    hb_buffer_add_utf8(buffer, text.c_str(), text.size(), 0, text.size());
    hb_buffer_guess_segment_properties(buffer);
    hb_shape(f.m_hb_font, buffer, nullptr, 0);

    unsigned int count = 0;
    auto* positions = hb_buffer_get_glyph_positions(buffer, &count);

    hb_position_t advance = 0;
    for (unsigned int i = 0; i < count; ++i) {
        advance += positions[i].x_advance;
    }

    hb_buffer_destroy(buffer);

    return static_cast<float>(advance) / 64.0f;
}

const Texture* Font::text_texture() { return m_text_texture.get(); }

const std::string& Font::font_path() { return m_font_path; }

} // namespace Cubed