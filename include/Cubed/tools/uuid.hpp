#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
namespace cubed {

class Uuid final {
public:
    using Storage = std::array<std::uint8_t, 16>;

    Uuid();
    constexpr explicit Uuid(Storage bytes) noexcept : m_bytes(bytes) {}

    [[nodiscard]] constexpr const Storage& bytes() const noexcept {
        return m_bytes;
    }

    [[nodiscard]] std::string to_proto_bytes() const;

    [[nodiscard]] std::string to_string() const;

    [[nodiscard]] static std::optional<Uuid>
    from_string(std::string_view value);

    bool operator==(const Uuid&) const = default;
    auto operator<=>(const Uuid&) const = default;

    static std::optional<Uuid> from_proto_bytes(std::string_view value);

    struct Hash {
        [[nodiscard]] std::size_t operator()(const Uuid& uuid) const noexcept;
    };

private:
    Storage m_bytes;
};

[[nodiscard]] std::string generate_uuid();

struct TransparentStringHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view value) const noexcept {
        return std::hash<std::string_view>{}(value);
    }

    std::size_t operator()(const std::string& value) const noexcept {
        return (*this)(std::string_view{value});
    }

    std::size_t operator()(const char* value) const noexcept {
        return (*this)(std::string_view{value});
    }
};

struct TransparentStringHashCompare {
    using is_transparent = void;

    static std::size_t hash(std::string_view value) noexcept {
        return std::hash<std::string_view>{}(value);
    }

    static std::size_t hash(const std::string& value) noexcept {
        return hash(std::string_view{value});
    }

    static bool equal(std::string_view lhs, std::string_view rhs) noexcept {
        return lhs == rhs;
    }
};

} // namespace cubed

template <> struct std::hash<cubed::Uuid> {
    std::size_t operator()(const cubed::Uuid& uuid) const noexcept {
        return cubed::Uuid::Hash{}(uuid);
    }
};
