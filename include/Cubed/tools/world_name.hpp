#pragma once

#include <string_view>

namespace cubed::tools {
[[nodiscard]] bool is_valid_world_name(std::string_view world_name);
}
