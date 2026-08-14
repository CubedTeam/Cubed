#include "Cubed/tools/uuid.hpp"

#include <bit>
#include <cstring>
#include <sodium.h>

namespace Cubed {

namespace {

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

std::string generate_uuid() { return Uuid{}.to_string(); }

} // namespace Cubed
