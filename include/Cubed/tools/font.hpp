#pragma once
#include "Cubed/render/texture.hpp"

#include <ft2build.h>
#include <memory>
#include FT_FREETYPE_H

#include "Cubed/primitive_data.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <hb-ft.h>
#include <string>
#include <unordered_map>

namespace Cubed {

struct Character {
    glm::vec2 uv_min;
    glm::vec2 uv_max;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

struct Glyph {
    glm::vec2 uv_min;
    glm::vec2 uv_max;

    glm::ivec2 size;
    glm::ivec2 bearing;

    int layer = 0;
};

struct TextMesh {
    std::vector<Vertex2D> vertices;

    float width;
    float height;

    float min_x;
    float min_y;
};

class Shader;

class Font {
public:
    static constexpr int CELL_SIZE = 64;
    Font();
    ~Font();
    static Font& get();

    static void destroy();
    TextMesh vertices(const std::string& text);
    const Texture* text_texture();
    static const std::string& font_path();
    static float text_width(const std::string& text);

private:
    FT_Library m_ft;
    FT_Face m_face;
    hb_font_t* m_hb_font;
    int m_next_layer = 0;
    float m_texture_width = 64;
    float m_texture_height = 64;

    std::unique_ptr<Texture> m_text_texture;
    static inline std::string m_font_path{ASSETS_PATH
                                          "fonts/unifont_t-17.0.05.otf"};
    std::unordered_map<uint32_t, Glyph> m_cache;
    int m_max_layers = 0;
    Glyph& load_glyph(uint32_t glyph_index);
    void upload_glyph(Glyph& glyph, const unsigned char* buffer);
    static std::unique_ptr<Font>& get_ptr();
};

} // namespace Cubed