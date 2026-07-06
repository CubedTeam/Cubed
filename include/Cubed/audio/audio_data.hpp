#pragma once
#include <cstdint>
#include <vector>
namespace Cubed {
struct AudioData {
    std::vector<int16_t> pcm;
    uint32_t channels;
    uint32_t sample_rate;
};
} // namespace Cubed
