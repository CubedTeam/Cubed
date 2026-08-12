#pragma once
#include "Cubed/gameplay/biome.hpp"
#include "Cubed/gameplay/block.hpp"
#include "Cubed/gameplay/chunk_pos.hpp"

#include <cstdint>
namespace rocksdb {
class DB;
}

namespace Cubed {

struct ChunkStorageData {
    ChunkPos pos{0, 0};

    uint32_t seed = 0;
    BiomeType biome = BiomeType::PLAIN;

    std::vector<BlockType> blocks;
};

class ChunkStorage {
public:
    static constexpr uint32_t VERSION = 1;
    explicit ChunkStorage(const std::filesystem::path& world_path);

    ~ChunkStorage();

    ChunkStorage(const ChunkStorage&) = delete;
    ChunkStorage& operator=(const ChunkStorage&) = delete;
    ChunkStorage(ChunkStorage&&) = delete;
    ChunkStorage& operator=(ChunkStorage&&) = delete;

    std::optional<ChunkStorageData> load(ChunkPos pos) const;

    bool save(const ChunkStorageData& chunk);

    bool save_batch(std::span<const ChunkStorageData> chunks,
                    bool sync = false);

    bool contains(ChunkPos pos) const;

    bool remove(ChunkPos pos);

private:
    std::unique_ptr<rocksdb::DB> m_db;

    static std::string make_key(ChunkPos pos);
    static std::string serialize(const ChunkStorageData& chunk);

    static std::optional<ChunkStorageData> deserialize(std::string_view data);
};

} // namespace Cubed
