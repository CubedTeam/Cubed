#pragma once
#include "Cubed/constants.hpp"
#include "Cubed/gameplay/biome.hpp"
#include "Cubed/gameplay/block.hpp"
#include "Cubed/gameplay/chunk_generator.hpp"
#include "Cubed/gameplay/chunk_pos.hpp"

#include <array>
#include <atomic>
#include <optional>
#include <tuple>
namespace Cubed {
class ServerWorld;
class ServerChunk {
public:
    ServerChunk(ServerWorld& world, ChunkPos chunk_pos,
                bool temp_chunk = false);
    ServerChunk(const ServerChunk&) = delete;
    ServerChunk(ServerChunk&&) noexcept;
    ServerChunk& operator=(const ServerChunk&) = delete;
    ServerChunk& operator=(ServerChunk&&) noexcept;

    static std::tuple<int, int, int> world_to_block(int world_x, int world_y,
                                                    int world_z, int chunk_x,
                                                    int chunk_z);
    static std::tuple<int, int, int> world_to_block(const glm::ivec3& block_pos,
                                                    ChunkPos chunk_pos);
    static std::tuple<int, int, int> block_to_world(int x, int y, int z,
                                                    int chunk_x, int chunk_z);
    static std::tuple<int, int, int> block_to_world(const glm::ivec3& block_pos,
                                                    ChunkPos chunk_pos);

    void set_chunk_block(int index, unsigned id);
    // ensure thread safe!
    void gen_chunk();

    BiomeType get_biome() const;
    ChunkPos get_chunk_pos() const;
    const std::vector<BlockType>& get_chunk_blocks() const;
    HeightMapArray get_heightmap() const;
    bool is_temp_chunk() const;
    ChunkPos chunk_pos() const;
    BiomeType biome() const;
    void biome(BiomeType b);
    HeightMapArray& heightmap();
    std::vector<BlockType>& blocks();
    ServerWorld& world();
    unsigned seed() const;
    BiomeConditions& conditions();
    bool& has_cave();
    const OptionalBlockVectorArray& get_neightbor_blocks() const;
    static int index(int x, int y, int z);
    static int index(const glm::vec3& pos);

private:
    static constexpr int SIZE_X = CHUNK_SIZE;
    static constexpr int SIZE_Y = WORLD_SIZE_Y;
    static constexpr int SIZE_Z = CHUNK_SIZE;

    std::atomic<bool> m_gening{false};
    std::atomic<bool> m_temp_chunk{false};

    bool m_has_cave{false};

    std::atomic<BiomeType> m_biome = BiomeType::PLAIN;

    ChunkPos m_chunk_pos;
    ServerWorld& m_world;
    HeightMapArray m_heightmap;
    // the index is a array of block id
    std::vector<BlockType> m_blocks;
    OptionalBlockVectorArray m_neightbor_blocks;
    float frequency = 0.01f;
    float height = 80;
    unsigned m_seed = 0;

    BiomeConditions m_conditions;

    std::unique_ptr<ChunkGenerator> m_generator;

    // Init Chunk
    // Determine biome from temperature and humidity noise
    void gen_phase_one();

    // Generate heightmap using biome-specific noise
    void gen_phase_two();

    // Generate terrain blocks from heightmap and biome
    void gen_phase_three();
    // Blend surface blocks at chunk borders with neighbors
    void gen_phase_four(const std::array<std::optional<std::vector<BlockType>>,
                                         4>& neighbor_block);
    // Generate biome-specific vegetation/structures
    void gen_phase_five();
};
} // namespace Cubed
