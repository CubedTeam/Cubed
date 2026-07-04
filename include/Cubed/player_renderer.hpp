#pragma once
#include "Cubed/shader.hpp"

#include <glad/glad.h>
namespace Cubed {
class Renderer;
class PlayerRenderer {
public:
    static constexpr int BODY_PART_NUM = 6;
    PlayerRenderer(Renderer& renderer);
    ~PlayerRenderer();
    void init();
    void render(const Shader& shader);
    void shadow_render(const Shader& shader, glm::mat4& light_matrix);

private:
    struct PlayerVertex {

        float x = 0.0f, y = 0.0f, z = 0.0f;
        float s = 0.0f, t = 0.0f;
        float nx = 0.0f, ny = 0.0f, nz = 0.0f;
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
    };
    Renderer& m_renderer;
    std::array<GLuint, BODY_PART_NUM> m_vao;
    std::array<GLuint, BODY_PART_NUM> m_vbo;
    bool m_inited{false};
    std::array<std::vector<PlayerVertex>, BODY_PART_NUM> m_vertices;
};
} // namespace Cubed
