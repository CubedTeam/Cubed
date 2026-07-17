#pragma once

#include "Cubed/input/event.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Cubed {

class ClientPlayer;

class Camera {
private:
    enum class Perspective {
        FIRST_PERSON,
        THIRD_PERSON_BACK,
        THIRD_PERSON_FRONT,
    };

    bool m_firse_mouse = true;
    ClientPlayer* m_player;
    float m_last_mouse_x, m_last_mouse_y;
    glm::vec3 m_camera_pos;
    bool m_under_water = false;
    Perspective m_perspective = Perspective::FIRST_PERSON;
    glm::vec3 m_front;
    glm::vec3 camera_collision(glm::vec3 start, glm::vec3 end,
                               float radius = 0.2f);

    bool handle_key_event(const KeyEvent& e);
    bool handle_mouse_move_event(const MouseMoveEvent& e);

public:
    Camera();

    void update_move_camera();

    void camera_init(ClientPlayer* player);
    void hot_reload();
    void reset_camera();
    void update_cursor_position_camera(double xpos, double ypos);

    const glm::mat4 get_camera_lookat() const;
    const glm::vec3& get_camera_pos() const;

    bool is_under_water() const;
    glm::vec3 get_camera_front() const;
    void change_perspective();
    bool is_first_person() const;
    bool handle_event(const Event& e);
};

} // namespace Cubed
