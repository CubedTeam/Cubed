#include "Cubed/tools/system_locate.hpp"

#ifdef _WIN32

#define NOMINMAX
#include <Windows.h>

namespace Cubed {

SystemLocale get_system_locale() {
    wchar_t locale_name[LOCALE_NAME_MAX_LENGTH]{};

    if (GetUserDefaultLocaleName(locale_name, LOCALE_NAME_MAX_LENGTH) == 0)
        return {};

    char utf8[LOCALE_NAME_MAX_LENGTH * 4]{};

    WideCharToMultiByte(CP_UTF8, 0, locale_name, -1, utf8, sizeof(utf8),
                        nullptr, nullptr);

    SystemLocale result;
    result.locale = utf8;

    auto pos = result.locale.find('-');
    if (pos != std::string::npos) {
        result.country = result.locale.substr(pos + 1);
        result.locale[pos] = '_'; // zh-CN -> zh_CN
    }

    return result;
}

} // namespace Cubed

#else

#include <cstdlib>
#include <string>

namespace Cubed {

SystemLocale get_system_locale() {
    SystemLocale result;

    const char* vars[] = {std::getenv("LC_ALL"), std::getenv("LC_MESSAGES"),
                          std::getenv("LANG")};

    for (const char* s : vars) {
        if (s && *s) {
            result.locale = s;
            break;
        }
    }

    if (result.locale.empty())
        return result;

    auto dot = result.locale.find('.');
    if (dot != std::string::npos)
        result.locale.erase(dot);

    auto at = result.locale.find('@');
    if (at != std::string::npos)
        result.locale.erase(at);

    auto pos = result.locale.find('_');
    if (pos != std::string::npos)
        result.country = result.locale.substr(pos + 1);

    return result;
}

} // namespace Cubed

#endif
