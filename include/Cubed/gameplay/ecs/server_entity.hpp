#pragma once
#include "Cubed/gameplay/ecs/entity.hpp"
#include "Cubed/gameplay/ecs/health.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/state.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/hitbox.hpp"
namespace Cubed {
struct ServerEntity {

    Entity entity;

    EntityInfo info;

    Transform transform;

    Velocity velocity;

    Movement movement;

    MoveState move_state;

    Direction direction;

    Gravity gravity;

    Health health;

    HitboxID hitbox;
};
} // namespace Cubed