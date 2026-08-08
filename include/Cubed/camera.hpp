#pragma once

#include "Cubed/input/event.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Cubed {

class LocalPlayer;

class Camera {
private:
    enum class Perspective {
        FIRST_PERSON,
        THIRD_PERSON_BACK,
        THIRD_PERSON_FRONT,
    };

    LocalPlayer* m_player = nullptr;
    float m_last_mouse_x = 0.0f, m_last_mouse_y = 0.0f;
    glm::vec3 m_camera_pos{0.0f};
    bool m_under_water = false;
    Perspective m_perspective = Perspective::FIRST_PERSON;
    glm::vec3 m_front{0.0f};
    glm::vec3 camera_collision(glm::vec3 start, glm::vec3 end,
                               float radius = 0.2f);

    bool handle_key_event(const KeyEvent& e);
    bool handle_mouse_move_event(const MouseMoveEvent& e);

public:
    Camera();

    void update_move_camera();

    void camera_init(LocalPlayer* player);
    void hot_reload();
    void update_cursor_position_camera(float offset_x, float offset_y);

    const glm::mat4 get_camera_lookat() const;
    const glm::vec3& get_camera_pos() const;

    bool is_under_water() const;
    glm::vec3 get_camera_front() const;
    void change_perspective();
    bool is_first_person() const;
    bool handle_event(const Event& e);
    LocalPlayer* player();
};

} // namespace Cubed
