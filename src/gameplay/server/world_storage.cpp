#include "Cubed/gameplay/server/world_storage.hpp"

#include "Cubed/tools/log.hpp"

#include <rocksdb/convenience.h>
#include <rocksdb/db.h>
#include <rocksdb/options.h>

namespace fs = std::filesystem;
namespace {

bool rocksdb_supports_zstd() {
    const auto COMPRESSIONS = rocksdb::GetSupportedCompressions();

    return std::find(COMPRESSIONS.begin(), COMPRESSIONS.end(),
                     rocksdb::kZSTD) != COMPRESSIONS.end();
}
} // namespace
namespace cubed {
WorldStorage::WorldStorage(const fs::path& db_path) {

    rocksdb::Options options;
    options.create_if_missing = true;

    if (!rocksdb_supports_zstd()) {
        throw std::runtime_error(
            "This RocksDB build does not support ZSTD compression");
    }

    options.compression = rocksdb::kZSTD;
    const auto DATABASE_PATH = db_path / "database";
    std::error_code ec;
    fs::create_directories(DATABASE_PATH.parent_path(), ec);

    if (ec) {
        throw std::runtime_error("Failed to create world directory " +
                                 DATABASE_PATH.parent_path().string() + ": " +
                                 ec.message());
    }

    Logger::info("World Database Path: {}", DATABASE_PATH.string());
    std::unique_ptr<rocksdb::DB> database = nullptr;

    auto status = rocksdb::DB::Open(options, DATABASE_PATH.string(), &database);

    if (!status.ok()) {
        throw std::runtime_error(
            std::format("Failed to open world database {}: {}",
                        DATABASE_PATH.string(), status.ToString()));
    }

    m_db = std::move(database);
};

WorldStorage::~WorldStorage() {}

rocksdb::DB* WorldStorage::get_db() { return m_db.get(); }

bool WorldStorage::save_metadata(const std::string& data) {
    auto status = m_db->Put(rocksdb::WriteOptions{}, make_metadata_key(), data);

    if (!status.ok()) {
        Logger::error("Failed to save metadata {}", status.ToString());
        return false;
    }
    return true;
}

std::optional<std::string> WorldStorage::get_metadata() {
    std::string value;
    auto status =
        m_db->Get(rocksdb::ReadOptions{}, make_metadata_key(), &value);
    if (status.IsNotFound()) {
        return std::nullopt;
    }
    if (!status.ok()) {
        Logger::error("Failed to read world metadata: {}", status.ToString());
        return std::nullopt;
    }
    return value;
}

std::string WorldStorage::make_metadata_key() { return "metadata:world"; }

} // namespace cubed
