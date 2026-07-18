#pragma once
#include <atomic>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <print>
#include <shared_mutex>
#include <string>
#include <syncstream>
#include <utility>
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
    enum class Level { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };
    static constexpr Level get_level(int i) {
        switch (i) {
        case std::to_underlying(Level::DEBUG):
            return Level::DEBUG;

        case std::to_underlying(Level::INFO):
            return Level::INFO;
        case std::to_underlying(Level::WARN):
            return Level::WARN;
        case std::to_underlying(Level::ERROR):
            return Level::ERROR;
        default:
            return Level::INFO;
        }
    }
    Logger() { m_file_stream = std::make_unique<std::ofstream>(); }
    ~Logger() { m_file_stream.reset(); }
    static Logger& instance() {
        static Logger inst;
        return inst;
    }
    static void set_level(Level level) { instance().m_level = level; }
    static void set_console_write(bool write) {
        instance().m_console_write = write;
    }

    static void set_file_write(bool write) { instance().m_file_write = write; }

    static void set_logs_path(const std::string& path) {
        std::unique_lock lock(instance().m_mutex);
        instance().m_logs_path = path;
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
        if (level < m_level) {
            return;
        }

        auto msg = std::format(fmt, std::forward<Args>(args)...);
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
        std::string message =
            std::format("{}[{:%Y-%m-%d %H:%M:%S}]{}", level_str, now_time, msg);

        if (m_file_write) {

            std::shared_lock lock{m_mutex};
            auto today = std::chrono::floor<std::chrono::days>(
                std::chrono::system_clock::now());
            auto date = std::chrono::year_month_day{today};

            if (!m_today.ok() || date > m_today) {
                lock.unlock();
                std::unique_lock l(m_mutex);

                auto today = std::chrono::floor<std::chrono::days>(
                    std::chrono::system_clock::now());
                auto date = std::chrono::year_month_day{today};

                if (!m_today.ok() || date > m_today) {
                    m_today = date;
                    try {
                        std::filesystem::create_directories(m_logs_path);
                        m_file_stream->close();
                        std::filesystem::path log =
                            m_logs_path / std::format("{:%Y-%m-%d}.log", date);
                        m_file_stream->open(log, std::ios::app);
                        if (!m_file_stream->is_open()) {
                            std::osyncstream(std::cerr)
                                << "File Stream Open Error\n";
                        }
                        // direct write
                        if (m_file_stream && m_file_stream->is_open()) {
                            std::osyncstream sync_out{*m_file_stream};
                            std::println(sync_out, "{}", message);
                        }
                    } catch (const std::exception& e) {
                        std::osyncstream(std::cerr)
                            << "Catch Logger Error " << e.what() << "\n";
                    }
                }
            } else {
                if (m_file_stream && m_file_stream->is_open()) {
                    std::osyncstream sync_out{*m_file_stream};
                    std::println(sync_out, "{}", message);
                }
            }
        }

        if (m_console_write) {
            std::osyncstream sync_out(level == Level::ERROR ? std::cerr
                                                            : std::cout);
            std::println(sync_out, "{}{}{}", color, message, CLEAR_ANSI);
        }
    }

private:
    static constexpr std::string_view CLEAR_ANSI = "\033[0m";
    static constexpr std::string_view INFO_ANSI = "\033[1;32m";
    static constexpr std::string_view ERROR_ANSI = "\033[1;31m";
    static constexpr std::string_view DEBUG_ANSI = "\033[1;34m";
    static constexpr std::string_view WARN_ANSI = "\033[1;33m";
    std::filesystem::path m_logs_path{"logs"};
    std::shared_mutex m_mutex;
    std::atomic<bool> m_file_write{false};
    std::atomic<bool> m_console_write{true};
    std::atomic<Level> m_level{Level::INFO};
    std::chrono::year_month_day m_today{};
    std::unique_ptr<std::ofstream> m_file_stream;
};

} // namespace Cubed
