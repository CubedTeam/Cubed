#pragma once
#include "Cubed/gameplay/ecs/health.hpp"
#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/hitbox.hpp"
namespace Cubed {

struct BaseServerCreature {
    Transform transform{};

    TickVelocity velocity{};

    Movement movement{};

    Gravity gravity{};

    Health health{};

    HitboxID hitbox{};
};

} // namespace Cubed