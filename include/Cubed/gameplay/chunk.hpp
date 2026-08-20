#pragma once
#include "Cubed/gameplay/chunk_pos.hpp"

#include <glm/glm.hpp>
#include <tuple>
namespace cubed {
class Chunk {
public:
    Chunk() = default;
    Chunk(const Chunk&) = delete;
    Chunk(Chunk&&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk& operator=(Chunk&&) = delete;
    virtual ~Chunk() = default;
    static int index(int x, int y, int z);
    static int index(const glm::vec3& pos);
    static std::tuple<int, int, int> world_to_block(int world_x, int world_y,
                                                    int world_z, int chunk_x,
                                                    int chunk_z);
    static std::tuple<int, int, int> world_to_block(const glm::ivec3& block_pos,
                                                    ChunkPos chunk_pos);
    static std::tuple<int, int, int> block_to_world(int x, int y, int z,
                                                    int chunk_x, int chunk_z);
    static std::tuple<int, int, int> block_to_world(const glm::ivec3& block_pos,
                                                    ChunkPos chunk_pos);
};
} // namespace cubed
