#pragma once

#include <type_traits>
namespace cubed {
template <typename T>
concept Ptr = std::is_pointer_v<T>;
}
