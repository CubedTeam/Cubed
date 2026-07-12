#include "Cubed/render/player_renderer.hpp"

#include "Cubed/camera.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/primitive_data.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/world_scene.hpp"
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
    {0, 32, 4, 12},  // front
    {4, 32, 4, 12},  // right
    {8, 32, 4, 12},  // back
    {12, 32, 4, 12}, // left
    {16, 32, 4, 4},  // top
    {20, 32, 4, 4},  // bottom
}};

constexpr FaceUV RIGHT_LEG_UV = {{
    {24, 32, 4, 12}, // front
    {28, 32, 4, 12}, // right
    {32, 32, 4, 12}, // back
    {36, 32, 4, 12}, // left
    {40, 32, 4, 4},  // top
    {44, 32, 4, 4},  // bottom
}};

constexpr std::array<std::array<UVRect, 6>,
                     Cubed::PlayerRenderer::BODY_PART_NUM>
    PLAYER_TEX = {{{HEAD_UV},
                   {BODY_UV},
                   {LEFT_ARM_UV},
                   {RIGHT_ARM_UV},
                   {LEFT_LEG_UV},
                   {RIGHT_LEG_UV}}};

constexpr glm::vec3 HEAD_PIVOT{0.5, 1.5, 0.5};
constexpr glm::vec3 LEFT_ARM_PIVOT{0.125f, 1.50f, 0.50f};
constexpr glm::vec3 RIGHT_ARM_PIVOT{0.875, 1.50, 0.50};
constexpr glm::vec3 LEFT_LEG_PIVOT{0.375, 0.75, 0.50};
constexpr glm::vec3 RIGHT_LEG_PIVOT{0.625, 0.75, 0.50};

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
                float su = TEX_COORDS[face][vex][0];
                float sv = TEX_COORDS[face][vex][1];

                if (face == 2) {
                    float t = su;
                    su = sv;
                    sv = 1.0f - t;
                }
                float u = rect.x + su * rect.w;
                float v = rect.y + sv * rect.h;

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

void PlayerRenderer::render(const Shader& shader, ClientWorld& world) {
    if (!m_inited) {
        Logger::error("Player Renderer isn't init");
        return;
    }

    auto& camera = world.world_scene().camera();
    auto& m_player = world.get_player();
    glm::mat4 m_v_mat = camera.get_camera_lookat();
    glm::mat4 m_p_mat = m_renderer.world_proj_matrix();

    auto& players = world.render_player_data();
    shader.set_loc("proj_matrix", m_p_mat);

    for (auto& player : players) {

        if (player.uuid == m_player.get_uuid()) {
            if (camera.is_first_person()) {
                continue;
            }
        }

        glm::mat4 model(1.0f);

        model = glm::translate(model, player.render_pos);

        // model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.5f));
        // glm::rotate(..., +yaw, Y) follows the OpenGL right‑handed coordinate
        // system, where a positive angle means counter‑clockwise rotation.
        // Therefore the model must use -yaw to align with the direction of
        // m_front.
        model = glm::rotate(model, glm::radians(-player.yaw + 180.0f),
                            glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));

        m_renderer.texture_mamger().get_skin()->bind(1);

        auto make_rotated = [&](glm::vec3 pivot, float angle) {
            glm::mat4 mat = model;
            mat = glm::translate(mat, pivot);
            mat = glm::rotate(mat, angle, glm::vec3(1, 0, 0));
            mat = glm::translate(mat, -pivot);
            return mat;
        };

        for (int i = 0; i < BODY_PART_NUM; i++) {
            switch (i) {
            case 0: {
                glm::mat4 head_model = model;
                head_model = glm::translate(head_model, HEAD_PIVOT);
                head_model =
                    glm::rotate(head_model, glm::radians(-player.pitch),
                                glm::vec3(1, 0, 0));
                head_model = glm::translate(head_model, -HEAD_PIVOT);
                glm::mat4 head_mv = m_v_mat * head_model;
                shader.set_loc("norm_matrix",
                               glm::transpose(glm::inverse(head_mv)));
                shader.set_loc("modelMatrix", head_model);
                shader.set_loc("mv_matrix", head_mv);
            } break;
            case 1: {
                glm::mat4 mv_mat = m_v_mat * model;
                shader.set_loc("norm_matrix",
                               glm::transpose(glm::inverse(mv_mat)));
                shader.set_loc("modelMatrix", model);
                shader.set_loc("mv_matrix", mv_mat);
            } break;
            case 2: { // left arm
                glm::mat4 model_mat =
                    make_rotated(LEFT_ARM_PIVOT, player.angle);
                glm::mat4 mv_mat = m_v_mat * model_mat;
                shader.set_loc("norm_matrix",
                               glm::transpose(glm::inverse(mv_mat)));
                shader.set_loc("modelMatrix", model_mat);
                shader.set_loc("mv_matrix", mv_mat);
            } break;
            case 3: { // right arm
                glm::mat4 model_mat =
                    make_rotated(RIGHT_ARM_PIVOT, -player.angle);
                glm::mat4 mv_mat = m_v_mat * model_mat;
                shader.set_loc("norm_matrix",
                               glm::transpose(glm::inverse(mv_mat)));
                shader.set_loc("modelMatrix", model_mat);
                shader.set_loc("mv_matrix", mv_mat);
            } break;
            case 4: { // left leg
                glm::mat4 model_mat =
                    make_rotated(LEFT_LEG_PIVOT, -player.angle);
                glm::mat4 mv_mat = m_v_mat * model_mat;
                shader.set_loc("norm_matrix",
                               glm::transpose(glm::inverse(mv_mat)));
                shader.set_loc("modelMatrix", model_mat);
                shader.set_loc("mv_matrix", mv_mat);
            } break;
            case 5: { // right leg
                glm::mat4 model_mat =
                    make_rotated(RIGHT_LEG_PIVOT, player.angle);

                glm::mat4 mv_mat = m_v_mat * model_mat;
                shader.set_loc("norm_matrix",
                               glm::transpose(glm::inverse(mv_mat)));
                shader.set_loc("modelMatrix", model_mat);
                shader.set_loc("mv_matrix", mv_mat);
            } break;
            }

            glBindVertexArray(m_vao[i]);
            glEnable(GL_DEPTH_TEST);
            glDrawArrays(GL_TRIANGLES, 0, m_vertices[i].size());
        }
    }
}

void PlayerRenderer::shadow_render(const Shader& shader,
                                   glm::mat4& light_matrix,
                                   ClientWorld& world) {
    if (!m_inited) {
        Logger::error("Player Renderer isn't init");
        return;
    }
    shader.use();
    shader.set_loc("lightSpaceMatrix", light_matrix);
    auto& players = world.render_player_data();

    for (auto& player : players) {
        glm::mat4 model(1.0f);

        model = glm::translate(model, player.render_pos);

        // model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.5f));

        model = glm::rotate(model, glm::radians(-player.yaw + 180.0f),
                            glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));

        auto make_rotated = [&](glm::vec3 pivot, float angle) {
            glm::mat4 mat = model;
            mat = glm::translate(mat, pivot);
            mat = glm::rotate(mat, angle, glm::vec3(1, 0, 0));
            mat = glm::translate(mat, -pivot);
            return mat;
        };

        for (int i = 0; i < BODY_PART_NUM; i++) {
            switch (i) {
            case 0: {
                glm::mat4 head_model = model;
                head_model = glm::translate(head_model, HEAD_PIVOT);
                head_model =
                    glm::rotate(head_model, glm::radians(-player.pitch),
                                glm::vec3(1, 0, 0));
                head_model = glm::translate(head_model, -HEAD_PIVOT);
                shader.set_loc("modelMatrix", head_model);
            } break;
            case 1: {
                shader.set_loc("modelMatrix", model);
            } break;
            case 2: { // left arm
                shader.set_loc("modelMatrix",
                               make_rotated(LEFT_ARM_PIVOT, player.angle));
            } break;
            case 3: { // right arm
                shader.set_loc("modelMatrix",
                               make_rotated(RIGHT_ARM_PIVOT, -player.angle));
            } break;
            case 4: { // left leg
                shader.set_loc("modelMatrix",

                               make_rotated(LEFT_LEG_PIVOT, -player.angle));
            } break;
            case 5: { // right leg
                shader.set_loc("modelMatrix",

                               make_rotated(RIGHT_LEG_PIVOT, player.angle));
            } break;
            }
            glBindVertexArray(m_vao[i]);
            glEnable(GL_DEPTH_TEST);
            glDrawArrays(GL_TRIANGLES, 0, m_vertices[i].size());
        }
    }
}

} // namespace Cubed