#pragma once

#include "Cubed/tools/cubed_concepts.hpp"
#include "common/vector3.pb.h"
#include "glm/ext/vector_float3.hpp"
namespace cubed {
namespace tools {
inline void set_proto_vec3(common::Vec3* p, const glm::vec3& pos) {
    p->set_x(pos.x);
    p->set_y(pos.y);
    p->set_z(pos.z);
}
template <Ptr T> void set_proto_pos(T ptr, const glm::vec3& pos) {
    set_proto_vec3(ptr->mutable_pos(), pos);
}

inline glm::vec3 get_proto_vec3(const common::Vec3* p) {
    return glm::vec3{p->x(), p->y(), p->z()};
}
inline glm::vec3 get_proto_vec3(const common::Vec3& p) {
    return glm::vec3{p.x(), p.y(), p.z()};
}
} // namespace tools
} // namespace cubed
