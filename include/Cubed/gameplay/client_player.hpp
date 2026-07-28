#pragma once

#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/player.hpp"
namespace Cubed {

struct ClientPlayer {
    Position pos{};
    Position render_pos{};
    EntityInfo entity{};
    WalkPose walk{};
    Orientation angle{};
    Orientation render_angle{};
};

struct PlayerRenderData {
    EntityInfo info{};
    Position render_pos{};
    Orientation angle{};
    Gait gait{};
};

} // namespace Cubed