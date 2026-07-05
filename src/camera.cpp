#include "Cubed/camera.hpp"

#include "Cubed/gameplay/client_player.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/tools/cubed_assert.hpp"

namespace {
constexpr float DISTANCE = 4.0f;
}
namespace Cubed {

Camera::Camera() {}

void Camera::update_move_camera() {
    ASSERT_MSG(m_player, "nullptr");
    auto pos = m_player->get_player_pos();
    // pos.y need to add 1.6f to center
    glm::vec3 eye = glm::vec3(pos.x, pos.y + 1.6f, pos.z);
    auto forward = m_player->get_front();
    m_front = forward;
    switch (m_perspective) {
    case Perspective::FIRST_PERSON:
        m_camera_pos = eye;
        break;
    case Perspective::THIRD_PERSON_BACK:
        m_camera_pos = eye - forward * DISTANCE;
        break;
    case Perspective::THIRD_PERSON_FRONT:
        m_camera_pos = eye + forward * DISTANCE;
        m_front = -m_front;
    }

    //  m_camera_pos += forward * 0.5f;
    glm::ivec3 block_pos = glm::floor(m_camera_pos);
    auto& world = m_player->get_world();
    if (world.get_block_tpye(block_pos) == 7) {
        m_under_water = true;
    } else {
        m_under_water = false;
    }
}

void Camera::camera_init(ClientPlayer* player) {
    m_player = player;
    update_move_camera();
    reset_camera();
    hot_reload();
}

void Camera::hot_reload() {}

void Camera::reset_camera() { m_firse_mouse = true; }

void Camera::update_cursor_position_camera(double xpos, double ypos) {
    if (m_firse_mouse) {
        m_last_mouse_x = xpos;
        m_last_mouse_y = ypos;
        m_firse_mouse = false;
        return;
    }

    float offset_x = xpos - m_last_mouse_x;
    float offset_y = m_last_mouse_y - ypos;

    m_last_mouse_x = xpos;
    m_last_mouse_y = ypos;
    ASSERT_MSG(m_player, "nullptr");
    m_player->update_front_vec(offset_x, offset_y);
}

const glm::mat4 Camera::get_camera_lookat() const {
    ASSERT_MSG(m_player, "nullptr");
    return glm::lookAt(m_camera_pos, m_camera_pos + m_front,
                       glm::vec3(0.0f, 1.0f, 0.0f));
}

const glm::vec3& Camera::get_camera_pos() const { return m_camera_pos; }

bool Camera::is_under_water() const { return m_under_water; }

glm::vec3 Camera::get_camera_front() const { return m_front; }

void Camera::change_perspective() {
    switch (m_perspective) {
    case Perspective::FIRST_PERSON:
        m_perspective = Perspective::THIRD_PERSON_BACK;
        break;
    case Perspective::THIRD_PERSON_BACK:
        m_perspective = Perspective::THIRD_PERSON_FRONT;
        break;
    case Perspective::THIRD_PERSON_FRONT:
        m_perspective = Perspective::FIRST_PERSON;
        break;
    }
}
bool Camera::is_first_person() const {
    return m_perspective == Perspective::FIRST_PERSON;
}
} // namespace Cubed
