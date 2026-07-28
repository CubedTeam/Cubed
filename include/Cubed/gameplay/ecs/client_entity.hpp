#pragma once
#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/ecs/entity.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/model.hpp"
namespace Cubed {
struct ClientEntity {

    Entity entity;

    EntityInfo info;

    Transform transform;

    WalkPose pose;

    ModelID model;
};
} // namespace Cubed