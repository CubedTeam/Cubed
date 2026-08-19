#pragma once
#include "Cubed/gameplay/biome.hpp"

#include <array>
#include <span>
#include <string_view>
namespace cubed {
struct SpawnConfig {
    std::string_view name;             // factory key, e.g. "cubed:pig"
    std::span<const BiomeType> biomes; // allowed biomes
    float probability = 0.0f;          // per chunk spawn probability
    unsigned max_spawn_count = 0;      // per chunk_max_spawn_sum
};

namespace spawn_defaults {
constexpr std::array<BiomeType, 2> PIG_BIOMES{BiomeType::PLAIN,
                                              BiomeType::FOREST};
constexpr SpawnConfig PIG{"cubed:pig", PIG_BIOMES, 0.02f, 3};
} // namespace spawn_defaults
} // namespace cubed
