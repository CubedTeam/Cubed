#pragma once
#include "Cubed/gameplay/entity.hpp"
namespace Cubed {
struct PlayerData {
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