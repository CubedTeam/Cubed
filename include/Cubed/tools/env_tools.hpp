#pragma once
#include <cstdlib> // IWYU pragma: keep
namespace Cubed {
namespace Tools {

inline void set_env(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}
} // namespace Tools
} // namespace Cubed