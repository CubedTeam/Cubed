#pragma once

#include <string_view>

namespace Cubed::Tools {
[[nodiscard]] bool is_valid_world_name(std::string_view world_name);
}
