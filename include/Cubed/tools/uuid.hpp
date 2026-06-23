#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
namespace Cubed {
inline std::string generate_uuid() {

    static std::mt19937_64 rng(
        std::chrono::steady_clock::now().time_since_epoch().count() ^
        (std::random_device{}()));
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    uint64_t a = dist(rng);
    uint64_t b = dist(rng);

    std::array<uint8_t, 16> bytes{};
    for (int i = 0; i < 8; ++i) {
        bytes[i] = (a >> (56 - 8 * i)) & 0xFF;
        bytes[8 + i] = (b >> (56 - 8 * i)) & 0xFF;
    }

    bytes[6] = (bytes[6] & 0x0F) | 0x40;

    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            ss << '-';
        }
        ss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return ss.str();
}
} // namespace Cubed