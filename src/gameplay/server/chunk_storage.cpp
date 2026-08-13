#include "Cubed/gameplay/server/chunk_storage.hpp"

#include "Cubed/gameplay/server/world_storage.hpp"
#include "save/stored_chunk.pb.h"

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/write_batch.h>
namespace fs = std::filesystem;
using namespace google::protobuf;
namespace Cubed {
ChunkStorage::ChunkStorage(WorldStorage& storage) : m_storage(storage) {}

ChunkStorage::~ChunkStorage() {}

bool ChunkStorage::save(const ChunkStorageData& chunk) {

    auto value = serialize(chunk);
    if (value.empty()) {
        return false;
    }

    auto status = m_storage.get_db()->Put(rocksdb::WriteOptions{},
                                          make_key(chunk.pos), value);

    if (!status.ok()) {
        Logger::error("Failed to save chunk {} {}: {}", chunk.pos.x,
                      chunk.pos.z, status.ToString());
        return false;
    }
    return true;
}

bool ChunkStorage::save_batch(std::span<const ChunkStorageData> chunks,
                              bool sync) {
    if (chunks.empty()) {
        return true;
    }

    rocksdb::WriteBatch batch;
    bool error = false;
    for (const auto& chunk : chunks) {
        auto value = serialize(chunk);

        if (value.empty()) {
            Logger::error("Failed to serialize chunk {} {}", chunk.pos.x,
                          chunk.pos.z);
            error = true;
            continue;
        }

        batch.Put(make_key(chunk.pos), value);
    }

    rocksdb::WriteOptions options;
    options.sync = sync;

    auto status = m_storage.get_db()->Write(options, &batch);

    if (!status.ok()) {
        Logger::error("Failed to save {} chunks: {}", chunks.size(),
                      status.ToString());
        return false;
    }

    return !error;
}

std::optional<ChunkStorageData> ChunkStorage::load(ChunkPos pos) const {
    std::string value;
    auto status =
        m_storage.get_db()->Get(rocksdb::ReadOptions{}, make_key(pos), &value);

    if (status.IsNotFound()) {
        return std::nullopt;
    }

    if (!status.ok()) {
        Logger::error("Failed to load chunk {} {}: {}", pos.x, pos.z,
                      status.ToString());
        return std::nullopt;
    }

    auto chunk = deserialize(value);

    if (!chunk) {
        Logger::error("Failed to deserialize chunk {} {}", pos.x, pos.z);
    }

    return chunk;
}

std::string ChunkStorage::make_key(ChunkPos pos) {
    return std::format("chunk:cubed:prime:{}:{}", pos.x, pos.z);
}

std::string ChunkStorage::serialize(const ChunkStorageData& chunk) {
    Arena arena;
    auto* p = Arena::Create<StoredChunk>(&arena);

    p->set_biome(std::to_underlying(chunk.biome));

    p->set_seed(chunk.seed);
    p->set_version(WorldStorage::VERSION);
    p->set_x(chunk.pos.x);
    p->set_z(chunk.pos.z);

    auto b = p->mutable_blocks();

    b->Assign(chunk.blocks.begin(), chunk.blocks.end());

    uint32_t raw_size = static_cast<uint32_t>(p->ByteSizeLong());
    std::string raw;
    raw.resize(raw_size);

    if (!p->SerializeToArray(raw.data(), raw_size)) {
        return {};
    }
    return raw;
}

std::optional<ChunkStorageData>
ChunkStorage::deserialize(std::string_view data) {

    Arena arena;
    auto* p = Arena::Create<StoredChunk>(&arena);

    if (!p->ParseFromArray(data.data(), static_cast<int>(data.size()))) {
        return std::nullopt;
    }

    if (p->version() > WorldStorage::VERSION) {
        Logger::error("Unsupported chunk version: {}", p->version());
        return std::nullopt;
    }

    if (p->blocks().size() != CHUNK_BLOCK_SUM) {
        Logger::error("Blocks sum {} != {}", p->blocks().size(),
                      CHUNK_BLOCK_SUM);
        return std::nullopt;
    }

    ChunkStorageData d;
    try {
        d.biome = get_biome_from_id(p->biome());
    } catch (const std::exception& e) {
        Logger::error("Get Biome Fail {}", e.what());
        return std::nullopt;
    }

    const auto& blocks = p->blocks();
    d.blocks.assign(blocks.begin(), blocks.end());
    d.pos.x = p->x();
    d.pos.z = p->z();
    d.seed = p->seed();

    return d;
}
/*
std::size_t ChunkStorage::size() const {
    std::size_t count = 0;

    std::unique_ptr<rocksdb::Iterator> it{
        m_storage.get_db()->NewIterator(rocksdb::ReadOptions{})};

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        ++count;
    }

    if (!it->status().ok()) {
        Logger::error("Failed to iterate chunk database: {}",
                      it->status().ToString());
        return 0;
    }

    return count;
}
*/
} // namespace Cubed