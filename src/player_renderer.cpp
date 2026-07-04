#include "Cubed/player_renderer.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/primitive_data.hpp"
#include "Cubed/renderer.hpp"
#include "Cubed/texture_manager.hpp"

#include <glm/glm.hpp>
namespace {
struct Cuboid {
    glm::vec3 offset;
    glm::vec3 size;
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

struct UVRect {
    int x;
    int y;
    int w;
    int h;
};
using FaceUV = std::array<UVRect, 6>;
constexpr FaceUV HEAD_UV = {{
    {0, 0, 8, 8},  // front
    {8, 0, 8, 8},  // right
    {16, 0, 8, 8}, // back
    {24, 0, 8, 8}, // left
    {32, 0, 8, 8}, // top
    {40, 0, 8, 8}, // bottom
}};

constexpr FaceUV BODY_UV = {{
    {0, 8, 8, 12},
    {8, 8, 4, 12},
    {12, 8, 8, 12},
    {20, 8, 4, 12},
    {24, 8, 8, 4},
    {32, 8, 8, 4},
}};

constexpr FaceUV LEFT_ARM_UV = {{
    {0, 20, 4, 12},
    {4, 20, 4, 12},
    {8, 20, 4, 12},
    {12, 20, 4, 12},
    {16, 20, 4, 4},
    {20, 20, 4, 4},
}};

constexpr FaceUV RIGHT_ARM_UV = {{
    {24, 20, 4, 12},
    {28, 20, 4, 12},
    {32, 20, 4, 12},
    {36, 20, 4, 12},
    {40, 20, 4, 4},
    {44, 20, 4, 4},
}};

constexpr FaceUV LEFT_LEG_UV = {{
    {0, 20, 4, 12},
    {4, 20, 4, 12},
    {8, 20, 4, 12},
    {12, 20, 4, 12},
    {16, 20, 4, 4},
    {20, 20, 4, 4},
}};

constexpr FaceUV RIGHT_LEG_UV = {{
    {24, 20, 4, 12},
    {28, 20, 4, 12},
    {32, 20, 4, 12},
    {36, 20, 4, 12},
    {40, 20, 4, 4},
    {44, 20, 4, 4},
}};

constexpr std::array<std::array<UVRect, 6>,
                     Cubed::PlayerRenderer::BODY_PART_NUM>
    PLAYER_TEX = {{{HEAD_UV},
                   {BODY_UV},
                   {LEFT_ARM_UV},
                   {RIGHT_ARM_UV},
                   {LEFT_LEG_UV},
                   {RIGHT_LEG_UV}}};

} // namespace
namespace Cubed {
PlayerRenderer::PlayerRenderer(Renderer& renderer) : m_renderer(renderer) {}

PlayerRenderer::~PlayerRenderer() {
    if (!m_inited) {
        return;
    }
    glDeleteBuffers(BODY_PART_NUM, m_vbo.data());
    glDeleteVertexArrays(BODY_PART_NUM, m_vao.data());
}

void PlayerRenderer::init() {

    for (int i = 0; i < 6; i++) {
        const auto& part = PLAYER_MODEL[i];

        for (int face = 0; face < 6; ++face) {
            const auto& rect = PLAYER_TEX[i][face];
            for (int vex = 0; vex < 6; ++vex) {
                glm::vec3 p{VERTICES_POS[face][vex][0],
                            VERTICES_POS[face][vex][1],
                            VERTICES_POS[face][vex][2]};

                float u = rect.x + TEX_COORDS[face][vex][0] * rect.w;
                float v = rect.y + TEX_COORDS[face][vex][1] * rect.h;

                u /= 64.0f;
                v /= 64.0f;

                p *= part.size;
                p += part.offset;

                m_vertices[i].emplace_back(
                    p.x, p.y, p.z, u, v, NORMALS[face][vex][0],
                    NORMALS[face][vex][1], NORMALS[face][vex][2],
                    TANGENTS[face][vex][0], TANGENTS[face][vex][1],
                    TANGENTS[face][vex][2]);
            }
        }
    }
    glGenVertexArrays(BODY_PART_NUM, m_vao.data());
    glGenBuffers(BODY_PART_NUM, m_vbo.data());
    for (int i = 0; i < BODY_PART_NUM; i++) {
        glBindVertexArray(m_vao[i]);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[i]);
        glBufferData(GL_ARRAY_BUFFER,
                     m_vertices[i].size() * sizeof(PlayerVertex),
                     m_vertices[i].data(), GL_STATIC_DRAW);

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
    }

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

        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        m_mv_mat = m_v_mat * model;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_renderer.texture_mamger().get_skin());

        for (int i = 0; i < BODY_PART_NUM; i++) {
            if (i == 0) {
                glm::mat4 head_model = model;

                head_model =
                    glm::translate(head_model, glm::vec3(0.5f, 1.5f, 0.5f));

                head_model =
                    glm::rotate(head_model, glm::radians(-player.pitch),
                                glm::vec3(-1, 0, 0));

                head_model =
                    glm::translate(head_model, glm::vec3(-0.5f, -1.5f, -0.5f));

                glm::mat4 head_mv = m_v_mat * head_model;
                shader.set_loc("mv_matrix", head_mv);
            } else {
                shader.set_loc("mv_matrix", m_mv_mat);
            }
            glBindVertexArray(m_vao[i]);
            glEnable(GL_DEPTH_TEST);
            glDrawArrays(GL_TRIANGLES, 0, m_vertices[i].size());
        }
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

        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));

        shader.set_loc("modelMatrix", model);

        for (int i = 0; i < BODY_PART_NUM; i++) {
            if (i == 0) {
                glm::mat4 head_model = model;

                head_model =
                    glm::translate(head_model, glm::vec3(0.5f, 1.5f, 0.5f));

                head_model =
                    glm::rotate(head_model, glm::radians(-player.pitch),
                                glm::vec3(-1, 0, 0));

                head_model =
                    glm::translate(head_model, glm::vec3(-0.5f, -1.5f, -0.5f));
                shader.set_loc("modelMatrix", head_model);
            } else {
                shader.set_loc("modelMatrix", model);
            }
            glBindVertexArray(m_vao[i]);
            glEnable(GL_DEPTH_TEST);
            glDrawArrays(GL_TRIANGLES, 0, m_vertices[i].size());
        }
    }
}

} // namespace Cubed