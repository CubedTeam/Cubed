#pragma once
#include "Cubed/gameplay/biome.hpp"
#include "Cubed/gameplay/block.hpp"
#include "Cubed/gameplay/chunk_pos.hpp"

#include <cstdint>

namespace Cubed {
class WorldStorage;
struct ChunkStorageData {
    ChunkPos pos{0, 0};

    uint32_t seed = 0;
    BiomeType biome = BiomeType::PLAIN;

    std::vector<BlockType> blocks;
};

class ChunkStorage {
public:
    explicit ChunkStorage(WorldStorage& storage);

    ~ChunkStorage();

    ChunkStorage(const ChunkStorage&) = delete;
    ChunkStorage& operator=(const ChunkStorage&) = delete;
    ChunkStorage(ChunkStorage&&) = delete;
    ChunkStorage& operator=(ChunkStorage&&) = delete;

    std::optional<ChunkStorageData> load(ChunkPos pos) const;

    bool save(const ChunkStorageData& chunk);

    bool save_batch(std::span<const ChunkStorageData> chunks,
                    bool sync = false);

    // bool contains(ChunkPos pos) const;

    // bool remove(ChunkPos pos);
    // std::size_t size() const;

private:
    WorldStorage& m_storage;

    static std::string make_key(ChunkPos pos);
    static std::string serialize(const ChunkStorageData& chunk);

    static std::optional<ChunkStorageData> deserialize(std::string_view data);
};

} // namespace Cubed
