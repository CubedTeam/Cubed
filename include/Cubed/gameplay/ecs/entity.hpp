#pragma once
#include <cstdint>
namespace Cubed {
using EntityID = uint64_t;

enum class EntityType { CREATURE, ITEM };

struct Entity {
    EntityID id;
    EntityType type;
    Entity(EntityID id, EntityType type) : id(id), type(type) {}
};

} // namespace Cubed