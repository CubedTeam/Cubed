#pragma once

#include "Cubed/constants.hpp"

#include <functional>

namespace Cubed {

struct ChunkPos {
    int x;
    int z;

    bool operator==(const ChunkPos&) const = default;
    struct Hash {
        std::size_t operator()(const ChunkPos& pos) const {
            std::size_t h1 = std::hash<int>{}(pos.x);
            std::size_t h2 = std::hash<int>{}(pos.z);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
    struct TBBHash {
        std::size_t hash(const ChunkPos& p) const {
            return ChunkPos::Hash{}(p);
        }
        bool equal(const ChunkPos& a, const ChunkPos& b) const {
            return a == b;
        }
    };
    ChunkPos operator+(const ChunkPos& pos) const {
        return ChunkPos{x + pos.x, z + pos.z};
    }

    ChunkPos& operator+=(const ChunkPos& pos) {
        x += pos.x;
        z += pos.z;
        return *this;
    };
};
constexpr ChunkPos CHUNK_DIR[]{{1, 0}, {-1, 0}, {0, 1},  {0, -1},
                               {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};
inline ChunkPos get_chunk_pos(int world_x, int world_z) {
    int chunk_x, chunk_z;
    if (world_x < 0) {
        chunk_x = (world_x + 1) / CHUNK_SIZE - 1;
    }
    if (world_x >= 0) {
        chunk_x = world_x / CHUNK_SIZE;
    }
    if (world_z < 0) {
        chunk_z = (world_z + 1) / CHUNK_SIZE - 1;
    }
    if (world_z >= 0) {
        chunk_z = world_z / CHUNK_SIZE;
    }
    return {chunk_x, chunk_z};
}

} // namespace Cubed