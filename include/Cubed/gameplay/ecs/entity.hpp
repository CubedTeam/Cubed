#pragma once
#include <cstdint>
namespace Cubed {
using EntityID = uint64_t;

struct Entity {
    EntityID id;
    explicit Entity(EntityID id) : id(id) {}
};

} // namespace Cubed