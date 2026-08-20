#pragma once
#include <cstdint>
namespace cubed {
using EntityID = uint64_t;

enum class EntityType { CREATURE, ITEM };

struct Entity {
    EntityID id;
    EntityType type;
    Entity(EntityID id, EntityType type) : id(id), type(type) {}
};

} // namespace cubed
