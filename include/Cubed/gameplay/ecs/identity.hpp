#pragma once

#include "Cubed/tools/uuid.hpp"

#include <optional>
#include <string>
namespace cubed {
struct EntityInfo {
    std::string name;
    std::optional<Uuid> uuid;
};
} // namespace cubed
