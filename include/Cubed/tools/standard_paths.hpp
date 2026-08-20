#pragma once

// A small, header-only StandardPaths helper for Windows and Linux.
// Requires C++20 or newer (C++23 is fine).

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
// clang-format off
#include <Windows.h>
#include <ShlObj.h>
// clang-format on
#ifdef _MSC_VER
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#endif
#elif defined(__linux__)
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#else
#error "StandardPaths currently supports only Windows and Linux"
#endif

namespace cubed {

class StandardPaths final {
public:
    // Config/Data/LocalData/Cache/State/Logs/Runtime are application-specific.
    // For example, on Linux:
    //   Config -> ~/.config/<organization>/<application>
    //   Data   -> ~/.local/share/<organization>/<application>
    enum class Location {
        HOME,
        DESKTOP,
        DOCUMENTS,
        DOWNLOADS,
        MUSIC,
        PICTURES,
        VIDEOS,
        TEMP,
        RUNTIME,
        CONFIG,
        DATA,
        LOCAL_DATA,
        CACHE,
        STATE,
        LOGS,
        EXECUTABLE,
        EXECUTABLE_DIRECTORY,
    };

    // organization may be empty; application must not be empty.
    // Both strings are UTF-8.
    explicit StandardPaths(std::string application,
                           std::string organization = {})
        : organization(validated_component(std::move(organization), true)),
          application(validated_component(std::move(application), false)) {}

    [[nodiscard]] std::filesystem::path
    writable_location(Location location) const {
        switch (location) {
        case Location::HOME:
            return home_directory();
        case Location::DESKTOP:
            return user_directory(UserDirectory::DESKTOP);
        case Location::DOCUMENTS:
            return user_directory(UserDirectory::DOCUMENTS);
        case Location::DOWNLOADS:
            return user_directory(UserDirectory::DOWNLOADS);
        case Location::MUSIC:
            return user_directory(UserDirectory::MUSIC);
        case Location::PICTURES:
            return user_directory(UserDirectory::PICTURES);
        case Location::VIDEOS:
            return user_directory(UserDirectory::VIDEOS);
        case Location::TEMP:
            return temp_directory();
        case Location::RUNTIME:
            return append_application(runtime_root());
        case Location::CONFIG:
            return append_application(config_root());
        case Location::DATA:
            return append_application(data_root());
        case Location::LOCAL_DATA:
            return append_application(local_data_root());
        case Location::CACHE:
            return cache_location();
        case Location::STATE:
            return state_location();
        case Location::LOGS:
            return logs_location();
        case Location::EXECUTABLE:
            return executable_path();
        case Location::EXECUTABLE_DIRECTORY:
            return executable_path().parent_path();
        }

        throw std::invalid_argument("unknown StandardPaths::Location");
    }

    // Creates the selected directory and returns it. Executable is a file and
    // therefore cannot be passed to ensure().
    [[nodiscard]] std::filesystem::path ensure(Location location) const {
        if (location == Location::EXECUTABLE) {
            throw std::invalid_argument(
                "cannot create the executable path as a directory");
        }

        const std::filesystem::path RESULT = writable_location(location);
        std::error_code error;
        std::filesystem::create_directories(RESULT, error);
        if (error) {
            throw std::system_error(error, "failed to create directory: " +
                                               RESULT.string());
        }

#ifdef __linux__
        // XDG_RUNTIME_DIR is required to be private to the current user.
        if (location == Location::RUNTIME) {
            std::filesystem::permissions(
                RESULT, std::filesystem::perms::owner_all,
                std::filesystem::perm_options::replace, error);
            if (error) {
                throw std::system_error(
                    error,
                    "failed to protect runtime directory: " + RESULT.string());
            }
        }
#endif
        return RESULT;
    }

    [[nodiscard]] static std::filesystem::path executable_path() {
#ifdef _WIN32
        std::vector<wchar_t> buffer(512);
        for (;;) {
            SetLastError(ERROR_SUCCESS);
            const DWORD length = GetModuleFileNameW(
                nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

            if (length == 0) {
                throw_last_windows_error("GetModuleFileNameW failed");
            }
            if (length < buffer.size() - 1) {
                return std::filesystem::path(
                    std::wstring(buffer.data(), length));
            }
            buffer.resize(buffer.size() * 2);
        }
#else
        std::vector<char> buffer(512);
        for (;;) {
            const ssize_t LENGTH =
                ::readlink("/proc/self/exe", buffer.data(), buffer.size());
            if (LENGTH < 0) {
                throw std::system_error(errno, std::generic_category(),
                                        "readlink(/proc/self/exe) failed");
            }
            if (static_cast<std::size_t>(LENGTH) < buffer.size()) {
                return std::filesystem::path(std::string(
                    buffer.data(), static_cast<std::size_t>(LENGTH)));
            }
            buffer.resize(buffer.size() * 2);
        }
#endif
    }

private:
    enum class UserDirectory {
        DESKTOP,
        DOCUMENTS,
        DOWNLOADS,
        MUSIC,
        PICTURES,
        VIDEOS,
    };

    std::filesystem::path organization;
    std::filesystem::path application;

    [[nodiscard]] std::filesystem::path
    append_application(const std::filesystem::path& root) const {
        if (organization.empty()) {
            return root / application;
        }
        return root / organization / application;
    }

    [[nodiscard]] std::filesystem::path cache_location() const {
#ifdef _WIN32
        return append_application(local_data_root()) / "cache";
#else
        return append_application(cache_root());
#endif
    }

    [[nodiscard]] std::filesystem::path state_location() const {
#ifdef _WIN32
        return append_application(local_data_root()) / "state";
#else
        return append_application(state_root());
#endif
    }

    [[nodiscard]] std::filesystem::path logs_location() const {
#ifdef _WIN32
        return append_application(local_data_root()) / "logs";
#else
        return append_application(state_root()) / "logs";
#endif
    }

    [[nodiscard]] static std::filesystem::path
    validated_component(std::string value, bool may_be_empty) {
        if (value.empty()) {
            if (may_be_empty) {
                return {};
            }
            throw std::invalid_argument("application name must not be empty");
        }

        if (value == "." || value == ".." ||
            value.find_first_of("<>:\"/\\|?*") != std::string::npos ||
            value.back() == ' ' || value.back() == '.') {
            throw std::invalid_argument("invalid portable path component: " +
                                        value);
        }
        if (std::any_of(value.begin(), value.end(),
                        [](unsigned char ch) { return ch < 0x20; })) {
            throw std::invalid_argument(
                "path component contains a control character");
        }

        // Windows treats these names as devices even when an extension exists.
        std::string stem = value.substr(0, value.find('.'));
        std::transform(stem.begin(), stem.end(), stem.begin(),
                       [](unsigned char ch) {
                           return static_cast<char>(std::toupper(ch));
                       });
        const bool RESERVED =
            stem == "CON" || stem == "PRN" || stem == "AUX" || stem == "NUL" ||
            (stem.size() == 4 &&
             (stem.starts_with("COM") || stem.starts_with("LPT")) &&
             stem[3] >= '1' && stem[3] <= '9');
        if (RESERVED) {
            throw std::invalid_argument("reserved Windows path component: " +
                                        value);
        }

        return path_from_utf8(value);
    }

    [[nodiscard]] static std::filesystem::path
    path_from_utf8(std::string_view value) {
#ifdef _WIN32
        if (value.empty()) {
            return {};
        }
        const int length =
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                static_cast<int>(value.size()), nullptr, 0);
        if (length == 0) {
            throw_last_windows_error("invalid UTF-8 path component");
        }
        std::wstring wide(static_cast<std::size_t>(length), L'\0');
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                static_cast<int>(value.size()), wide.data(),
                                length) == 0) {
            throw_last_windows_error("UTF-8 path conversion failed");
        }
        return std::filesystem::path(std::move(wide));
#else
        return std::filesystem::path(value);
#endif
    }

#ifdef _WIN32
    [[noreturn]] static void throw_last_windows_error(const char* message) {
        throw std::system_error(static_cast<int>(GetLastError()),
                                std::system_category(), message);
    }

    [[nodiscard]] static std::filesystem::path
    known_folder(REFKNOWNFOLDERID id) {
        PWSTR raw = nullptr;
        const HRESULT result =
            SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, nullptr, &raw);
        if (FAILED(result)) {
            throw std::system_error(static_cast<int>(result),
                                    std::system_category(),
                                    "SHGetKnownFolderPath failed");
        }

        std::filesystem::path path(raw);
        CoTaskMemFree(raw);
        return path;
    }

    [[nodiscard]] static std::filesystem::path home_directory() {
        return known_folder(FOLDERID_Profile);
    }

    [[nodiscard]] static std::filesystem::path
    user_directory(UserDirectory directory) {
        switch (directory) {
        case UserDirectory::DESKTOP:
            return known_folder(FOLDERID_Desktop);
        case UserDirectory::DOCUMENTS:
            return known_folder(FOLDERID_Documents);
        case UserDirectory::DOWNLOADS:
            return known_folder(FOLDERID_Downloads);
        case UserDirectory::MUSIC:
            return known_folder(FOLDERID_Music);
        case UserDirectory::PICTURES:
            return known_folder(FOLDERID_Pictures);
        case UserDirectory::VIDEOS:
            return known_folder(FOLDERID_Videos);
        }
        throw std::invalid_argument("unknown user directory");
    }

    [[nodiscard]] static std::filesystem::path temp_directory() {
        std::vector<wchar_t> buffer(MAX_PATH + 1);
        for (;;) {
            const DWORD length =
                GetTempPathW(static_cast<DWORD>(buffer.size()), buffer.data());
            if (length == 0) {
                throw_last_windows_error("GetTempPathW failed");
            }
            if (length < buffer.size()) {
                return std::filesystem::path(
                    std::wstring(buffer.data(), length));
            }
            buffer.resize(static_cast<std::size_t>(length) + 1);
        }
    }

    [[nodiscard]] static std::filesystem::path config_root() {
        return known_folder(FOLDERID_RoamingAppData);
    }
    [[nodiscard]] static std::filesystem::path data_root() {
        return known_folder(FOLDERID_RoamingAppData);
    }
    [[nodiscard]] static std::filesystem::path local_data_root() {
        return known_folder(FOLDERID_LocalAppData);
    }
    [[nodiscard]] static std::filesystem::path runtime_root() {
        return temp_directory() / "runtime";
    }

#else
    [[nodiscard]] static std::optional<std::filesystem::path>
    absolute_environment_path(const char* name) {
        const char* value = std::getenv(name);
        if (value == nullptr || *value == '\0') {
            return std::nullopt;
        }
        std::filesystem::path path(value);
        if (!path.is_absolute()) {
            return std::nullopt;
        }
        return path;
    }

    [[nodiscard]] static std::filesystem::path home_directory() {
        if (auto path = absolute_environment_path("HOME")) {
            return *path;
        }

        long suggested_size = ::sysconf(_SC_GETPW_R_SIZE_MAX);
        if (suggested_size < 1024) {
            suggested_size = 16384;
        }
        std::vector<char> buffer(static_cast<std::size_t>(suggested_size));
        passwd entry{};
        passwd* result = nullptr;
        const int ERROR = ::getpwuid_r(::getuid(), &entry, buffer.data(),
                                       buffer.size(), &result);
        if (ERROR != 0) {
            throw std::system_error(ERROR, std::generic_category(),
                                    "getpwuid_r failed");
        }
        if (result == nullptr || result->pw_dir == nullptr ||
            *result->pw_dir == '\0') {
            throw std::runtime_error(
                "the current user's home directory is unavailable");
        }
        return std::filesystem::path(result->pw_dir);
    }

    [[nodiscard]] static std::filesystem::path
    xdg_root(const char* variable, const std::filesystem::path& fallback) {
        if (auto path = absolute_environment_path(variable)) {
            return *path;
        }
        return home_directory() / fallback;
    }

    [[nodiscard]] static std::filesystem::path config_root() {
        return xdg_root("XDG_CONFIG_HOME", ".config");
    }
    [[nodiscard]] static std::filesystem::path data_root() {
        return xdg_root("XDG_DATA_HOME",
                        std::filesystem::path(".local") / "share");
    }
    [[nodiscard]] static std::filesystem::path local_data_root() {
        return data_root();
    }
    [[nodiscard]] static std::filesystem::path cache_root() {
        return xdg_root("XDG_CACHE_HOME", ".cache");
    }
    [[nodiscard]] static std::filesystem::path state_root() {
        return xdg_root("XDG_STATE_HOME",
                        std::filesystem::path(".local") / "state");
    }
    [[nodiscard]] static std::filesystem::path temp_directory() {
        std::error_code error;
        std::filesystem::path result =
            std::filesystem::temp_directory_path(error);
        if (error) {
            throw std::system_error(error,
                                    "failed to find the temporary directory");
        }
        return result;
    }
    [[nodiscard]] static std::filesystem::path runtime_root() {
        if (auto path = absolute_environment_path("XDG_RUNTIME_DIR")) {
            return *path;
        }
        // Useful outside a graphical/login session. ensure(Runtime) changes the
        // final application directory to mode 0700.
        return temp_directory() /
               ("runtime-" +
                std::to_string(static_cast<unsigned long>(::getuid())));
    }

    [[nodiscard]] static std::string trim(std::string value) {
        const auto NOT_SPACE = [](unsigned char ch) {
            return !std::isspace(ch);
        };
        value.erase(value.begin(),
                    std::find_if(value.begin(), value.end(), NOT_SPACE));
        value.erase(
            std::find_if(value.rbegin(), value.rend(), NOT_SPACE).base(),
            value.end());
        return value;
    }

    [[nodiscard]] static std::optional<std::filesystem::path>
    parse_xdg_user_directory(std::string_view key) {
        std::ifstream input(config_root() / "user-dirs.dirs");
        if (!input) {
            return std::nullopt;
        }

        const std::string EXPECTED = "XDG_" + std::string(key) + "_DIR";
        std::string line;
        while (std::getline(input, line)) {
            line = trim(std::move(line));
            if (line.empty() || line[0] == '#') {
                continue;
            }

            const std::size_t EQUALS = line.find('=');
            if (EQUALS == std::string::npos ||
                trim(line.substr(0, EQUALS)) != EXPECTED) {
                continue;
            }

            std::string value = trim(line.substr(EQUALS + 1));
            if (value.size() >= 2 && value.front() == '"' &&
                value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            const std::filesystem::path HOME = home_directory();
            if (value == "$HOME" || value == "${HOME}") {
                return HOME;
            }
            if (value.starts_with("$HOME/")) {
                value.erase(0, 6);
                return HOME / std::filesystem::path(value);
            }
            if (value.starts_with("${HOME}/")) {
                value.erase(0, 8);
                return HOME / std::filesystem::path(value);
            }

            std::filesystem::path result(value);
            if (result.is_absolute()) {
                return result;
            }
            return std::nullopt;
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::filesystem::path
    user_directory(UserDirectory directory) {
        std::string_view key;
        std::filesystem::path fallback;
        switch (directory) {
        case UserDirectory::DESKTOP:
            key = "DESKTOP";
            fallback = "Desktop";
            break;
        case UserDirectory::DOCUMENTS:
            key = "DOCUMENTS";
            fallback = "Documents";
            break;
        case UserDirectory::DOWNLOADS:
            key = "DOWNLOAD";
            fallback = "Downloads";
            break;
        case UserDirectory::MUSIC:
            key = "MUSIC";
            fallback = "Music";
            break;
        case UserDirectory::PICTURES:
            key = "PICTURES";
            fallback = "Pictures";
            break;
        case UserDirectory::VIDEOS:
            key = "VIDEOS";
            fallback = "Videos";
            break;
        }

        if (auto path = parse_xdg_user_directory(key)) {
            return *path;
        }
        return home_directory() / fallback;
    }
#endif
};

} // namespace cubed
