#pragma once
#include "Cubed/shader.hpp"

#include <glad/glad.h>
namespace Cubed {
class Renderer;
class PlayerRenderer {
public:
    PlayerRenderer(Renderer& renderer);
    ~PlayerRenderer();
    void init();
    void render(const Shader& shader);
    void shadow_render(const Shader& shader, glm::mat4& light_matrix);
    GLuint vao();
    int sum();

private:
    Renderer& m_renderer;
    GLuint m_vao;
    GLuint m_vbo;
    int m_vertices_sum = 0;
    bool m_inited{false};
};
} // namespace Cubed
