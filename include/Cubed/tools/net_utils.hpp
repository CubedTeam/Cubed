#pragma once

#include "Cubed/tools/cubed_concepts.hpp"
#include "common/vector3.pb.h"
#include "glm/ext/vector_float3.hpp"
namespace Cubed {
namespace Tools {
template <Ptr T> void set_net_pos(T ptr, const glm::vec3& pos) {
    Vec3* p = ptr->mutable_pos();
    p->set_x(pos.x);
    p->set_y(pos.y);
    p->set_z(pos.z);
}
} // namespace Tools
} // namespace Cubed