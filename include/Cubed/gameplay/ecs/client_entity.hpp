#pragma once
#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/model.hpp"
namespace Cubed {
struct BaseClientCreature {

    Transform transform;

    WalkPose pose;

    ModelID model;
};
} // namespace Cubed