#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <print>
#include <string>
#include <syncstream>
#ifdef DEBUG
#undef DEBUG
#endif

#ifdef INFO
#undef INFO
#endif

#ifdef ERROR
#undef ERROR
#endif

#ifdef WARN
#undef WARN
#endif

namespace Cubed {

class Logger {
public:
    enum class Level { DEBUG, INFO, ERROR, WARN };
    Logger() {}
    static Logger& instance() {
        static Logger inst;
        return inst;
    }
    template <typename... Args>
    static void info(std::format_string<Args...> fmt, Args&&... args) {
        instance().log(Level::INFO, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void error(std::format_string<Args...> fmt, Args&&... args) {
        instance().log(Level::ERROR, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void warn(std::format_string<Args...> fmt, Args&&... args) {
        instance().log(Level::WARN, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void debug(std::format_string<Args...> fmt, Args&&... args) {
        instance().log(Level::DEBUG, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    void log(Level level, std::format_string<Args...> fmt, Args&&... args) {
        auto now_time = std::chrono::time_point_cast<std::chrono::seconds>(
            std::chrono::system_clock::now());

        std::string msg =
            std::vformat(fmt.get(), std::make_format_args(args...));
        std::string_view level_str;
        std::string_view color;
        switch (level) {
        case Level::INFO:
            color = INFO_ANSI;
            level_str = "[INFO]";
            break;
        case Level::ERROR:
            color = ERROR_ANSI;
            level_str = "[ERROR]";
            break;
        case Level::WARN:
            color = WARN_ANSI;
            level_str = "[WARN]";
            break;
        case Level::DEBUG:
            color = DEBUG_ANSI;
            level_str = "[DEBUG]";
            break;
        }
        std::osyncstream sync_out(level == Level::ERROR ? std::cerr
                                                        : std::cout);
        std::println(sync_out, "{}{}[{:%Y-%m-%d %H:%M:%S}]{}{}", color,
                     level_str, now_time, msg, CLEAR_ANSI);
    }

private:
    static constexpr std::string_view CLEAR_ANSI = "\033[0m";
    static constexpr std::string_view INFO_ANSI = "\033[1;32m";
    static constexpr std::string_view ERROR_ANSI = "\033[1;31m";
    static constexpr std::string_view DEBUG_ANSI = "\033[1;34m";
    static constexpr std::string_view WARN_ANSI = "\033[1;33m";
};

} // namespace Cubed
