#pragma once
#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/model.hpp"

#include <deque>
namespace cubed {
struct BaseClientCreature {

    Transform transform{};

    WalkPose pose{};

    ModelID model = 0;
};

struct SoundTime {
    float next_call_time = 0.0f;
};

struct ClientEntitySnapshot {
    double time_ms = 0.0;
    glm::vec3 pos{0.0f};
    glm::vec3 dir{0.0f};
};

struct ClientEntityState {
    std::deque<ClientEntitySnapshot> history;
};

} // namespace cubed
