#include "Cubed/tools/uuid.hpp"

#include <algorithm>
#include <bit>
#include <cstring>
#include <sodium.h>
namespace Cubed {

namespace {

std::optional<std::uint8_t> hex_value(char value) noexcept {
    if (value >= '0' && value <= '9') {
        return static_cast<std::uint8_t>(value - '0');
    }
    if (value >= 'a' && value <= 'f') {
        return static_cast<std::uint8_t>(value - 'a' + 10);
    }
    if (value >= 'A' && value <= 'F') {
        return static_cast<std::uint8_t>(value - 'A' + 10);
    }
    return std::nullopt;
}

std::uint64_t mix(std::uint64_t value) noexcept {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

} // namespace

Uuid::Uuid() {
    // AI-generated: Create an RFC 4122 version 4 UUID.
    randombytes_buf(m_bytes.data(), m_bytes.size());
    m_bytes[6] = static_cast<std::uint8_t>((m_bytes[6] & 0x0fU) | 0x40U);
    m_bytes[8] = static_cast<std::uint8_t>((m_bytes[8] & 0x3fU) | 0x80U);
}

std::string Uuid::to_string() const {
    static constexpr char HEX[] = "0123456789abcdef";
    std::string result(36, '-');
    std::size_t output = 0;

    for (std::size_t i = 0; i < m_bytes.size(); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            ++output;
        }
        result[output++] = HEX[m_bytes[i] >> 4];
        result[output++] = HEX[m_bytes[i] & 0x0fU];
    }

    return result;
}

std::optional<Uuid> Uuid::from_string(std::string_view value) {
    if (value.size() != 36 || value[8] != '-' || value[13] != '-' ||
        value[18] != '-' || value[23] != '-') {
        return std::nullopt;
    }

    // AI-generated: Decode the canonical UUID text format.
    Storage bytes{};
    std::size_t input = 0;
    for (auto& byte : bytes) {
        if (input == 8 || input == 13 || input == 18 || input == 23) {
            ++input;
        }

        const auto high = hex_value(value[input++]);
        const auto low = hex_value(value[input++]);
        if (!high || !low) {
            return std::nullopt;
        }
        byte = static_cast<std::uint8_t>((*high << 4U) | *low);
    }

    return Uuid{bytes};
}

std::size_t Uuid::Hash::operator()(const Uuid& uuid) const noexcept {
    std::uint64_t first;
    std::uint64_t second;
    std::memcpy(&first, uuid.bytes().data(), sizeof(first));
    std::memcpy(&second, uuid.bytes().data() + sizeof(first), sizeof(second));

    const std::uint64_t HASH = mix(first) ^ std::rotl(mix(second), 1);
    if constexpr (sizeof(std::size_t) < sizeof(HASH)) {
        return static_cast<std::size_t>(HASH ^ (HASH >> 32));
    }
    return static_cast<std::size_t>(HASH);
}

std::optional<Cubed::Uuid>
Uuid::uuid_from_proto_bytes(const std::string& value) {

    if (value.size() != 16) {
        return std::nullopt;
    }

    Storage bytes{};
    std::copy_n(reinterpret_cast<const std::uint8_t*>(value.data()),
                bytes.size(), bytes.begin());

    return Cubed::Uuid{bytes};
}

std::string generate_uuid() { return Uuid{}.to_string(); }

} // namespace Cubed
