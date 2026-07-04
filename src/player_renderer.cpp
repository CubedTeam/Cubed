#include "Cubed/player_renderer.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/primitive_data.hpp"
#include "Cubed/renderer.hpp"

#include <glm/glm.hpp>
namespace {
struct Cuboid {
    glm::vec3 offset;
    glm::vec3 size;
};

struct PlayerVertex {

    float x = 0.0f, y = 0.0f, z = 0.0f;
    float s = 0.0f, t = 0.0f;
    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
    float tx = 0.0f, ty = 0.0f, tz = 0.0f;
};

constexpr Cuboid PLAYER_MODEL[] = {
    // Head
    {{0.25f, 1.50f, 0.25f}, {0.50f, 0.50f, 0.50f}},

    // Body
    {{0.25f, 0.75f, 0.375f}, {0.50f, 0.75f, 0.25f}},

    // Left Arm
    {{0.00f, 0.75f, 0.375f}, {0.25f, 0.75f, 0.25f}},

    // Right Arm
    {{0.75f, 0.75f, 0.375f}, {0.25f, 0.75f, 0.25f}},

    // Left Leg
    {{0.25f, 0.00f, 0.375f}, {0.25f, 0.75f, 0.25f}},

    // Right Leg
    {{0.50f, 0.00f, 0.375f}, {0.25f, 0.75f, 0.25f}},
};

} // namespace
namespace Cubed {
PlayerRenderer::PlayerRenderer(Renderer& renderer) : m_renderer(renderer) {}

PlayerRenderer::~PlayerRenderer() {
    if (!m_inited) {
        return;
    }
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void PlayerRenderer::init() {
    std::vector<PlayerVertex> vertices;
    for (const auto& part : PLAYER_MODEL) {
        for (int face = 0; face < 6; ++face) {
            for (int v = 0; v < 6; ++v) {
                glm::vec3 p{VERTICES_POS[face][v][0], VERTICES_POS[face][v][1],
                            VERTICES_POS[face][v][2]};

                p *= part.size;
                p += part.offset;
                vertices.emplace_back(p.x, p.y, p.z, TEX_COORDS[face][v][0],
                                      TEX_COORDS[face][v][1],
                                      NORMALS[face][v][0], NORMALS[face][v][1],
                                      NORMALS[face][v][2], TANGENTS[face][v][0],
                                      TANGENTS[face][v][1],
                                      TANGENTS[face][v][2]);
            }
        }
    }
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PlayerVertex),
                 vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PlayerVertex),
                          (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(PlayerVertex),
                          (void*)offsetof(PlayerVertex, s));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PlayerVertex),
                          (void*)offsetof(PlayerVertex, nx));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PlayerVertex),
                          (void*)offsetof(PlayerVertex, tx));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_vertices_sum = vertices.size();
    m_inited = true;
}

void PlayerRenderer::render(const Shader& shader) {
    if (!m_inited) {
        Logger::error("Player Renderer isn't init");
        return;
    }
    auto& m_camera = m_renderer.camera();
    auto& m_world = m_renderer.world();
    auto& m_player = m_world.get_player();
    glm::mat4 m_v_mat = m_camera.get_camera_lookat();
    glm::mat4 m_mv_mat;
    glm::mat4 m_p_mat = m_renderer.proj_mat();

    auto& players = m_world.render_player_data();
    shader.set_loc("proj_matrix", m_p_mat);

    for (auto& player : players) {
        if (player.uuid == m_player.get_uuid()) {
            continue;
        }
        glm::mat4 model(1.0f);

        model = glm::translate(model, player.render_pos);

        model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.5f));

        model = glm::rotate(model, glm::radians(player.yaw),
                            glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));
        m_mv_mat = m_v_mat * model;

        shader.set_loc("mv_matrix", m_mv_mat);
        glBindVertexArray(m_vao);
        glEnable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, m_vertices_sum);
    }
}

void PlayerRenderer::shadow_render(const Shader& shader,
                                   glm::mat4& light_matrix) {
    if (!m_inited) {
        Logger::error("Player Renderer isn't init");
        return;
    }
    shader.use();
    shader.set_loc("lightSpaceMatrix", light_matrix);
    auto& m_world = m_renderer.world();
    auto& players = m_world.render_player_data();

    for (auto& player : players) {
        glm::mat4 model(1.0f);

        model = glm::translate(model, player.render_pos);

        model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.5f));

        model = glm::rotate(model, glm::radians(player.yaw),
                            glm::vec3(0.0f, 1.0f, 0.0f));

        model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));

        shader.set_loc("modelMatrix", model);
        glBindVertexArray(m_vao);
        glEnable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, m_vertices_sum);
    }
}

GLuint PlayerRenderer::vao() { return m_vao; }
int PlayerRenderer::sum() { return m_vertices_sum; }
} // namespace Cubed