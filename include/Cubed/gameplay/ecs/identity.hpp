#pragma once

#include "Cubed/tools/uuid.hpp"

#include <optional>
#include <string>
namespace Cubed {
struct EntityInfo {
    std::string name;
    std::optional<Uuid> uuid;
};
} // namespace Cubed