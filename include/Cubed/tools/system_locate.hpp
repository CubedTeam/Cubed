#pragma once

#include <string>

namespace cubed {
struct SystemLocale {
    std::string country; // CN、US、JP...
    std::string locale;  // zh_CN、en_US...
};

SystemLocale get_system_locale();

} // namespace cubed
