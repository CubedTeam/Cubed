#pragma once

#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"

#include <deque>
namespace cubed {

struct ClientPlayerSnapshot {
    double time_ms = 0.0f;
    glm::vec3 pos{0.0f};
    float yaw = 0.0f;
    float pitch = 0.0f;
};
struct ClientPlayerState {
    std::deque<ClientPlayerSnapshot> value;
};
struct ClientPlayer {
    Position pos{};
    EntityInfo entity{};
    WalkPose walk{};
    Orientation angle{};
    Position render_pos{};
    Orientation render_angle{};
    ClientPlayerState history{};
};

struct PlayerRenderData {
    EntityInfo info{};
    Position render_pos{};
    Orientation angle{};
    Gait gait{};
};

} // namespace cubed
