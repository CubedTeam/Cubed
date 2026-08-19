#include "Cubed/gameplay/chunk.hpp"

#include "Cubed/tools/cubed_assert.hpp"

namespace cubed {
int Chunk::index(int x, int y, int z) {
    ASSERT(!(x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= WORLD_SIZE_Y ||
             z >= CHUNK_SIZE));
    if ((x * WORLD_SIZE_Y + y) * CHUNK_SIZE + z < 0 ||
        (x * WORLD_SIZE_Y + y) * CHUNK_SIZE + z >=
            CHUNK_SIZE * CHUNK_SIZE * WORLD_SIZE_Y) {
        Logger::error("block pos x {} y {} z {} range error", x, y, z);
        ASSERT(0);
    }
    return (x * WORLD_SIZE_Y + y) * CHUNK_SIZE + z;
}

int Chunk::index(const glm::vec3& pos) {
    return Chunk::index(pos.x, pos.y, pos.z);
}
std::tuple<int, int, int> Chunk::world_to_block(int world_x, int world_y,
                                                int world_z, int chunk_x,
                                                int chunk_z) {
    int x, y, z;
    y = world_y;
    x = world_x - chunk_x * CHUNK_SIZE;
    z = world_z - chunk_z * CHUNK_SIZE;
    return {x, y, z};
}

std::tuple<int, int, int> Chunk::world_to_block(const glm::ivec3& block_pos,
                                                ChunkPos chunk_pos) {
    return world_to_block(block_pos.x, block_pos.y, block_pos.z, chunk_pos.x,
                          chunk_pos.z);
}

std::tuple<int, int, int> Chunk::block_to_world(int x, int y, int z,
                                                int chunk_x, int chunk_z) {
    int world_x = x + chunk_x * CHUNK_SIZE;
    int world_z = z + chunk_z * CHUNK_SIZE;
    int world_y = y;
    return {world_x, world_y, world_z};
}
std::tuple<int, int, int> Chunk::block_to_world(const glm::ivec3& block_pos,
                                                ChunkPos chunk_pos) {
    return block_to_world(block_pos.x, block_pos.y, block_pos.z, chunk_pos.x,
                          chunk_pos.z);
}
} // namespace cubed
