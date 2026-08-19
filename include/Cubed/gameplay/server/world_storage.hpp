#pragma once
#include <filesystem>

namespace rocksdb {
class DB;
}
namespace cubed {
class WorldStorage {
public:
    static constexpr uint32_t VERSION = 1;
    WorldStorage(const WorldStorage&) = delete;
    WorldStorage(WorldStorage&&) = delete;
    WorldStorage& operator=(const WorldStorage&) = delete;
    WorldStorage& operator=(WorldStorage&&) = delete;

    explicit WorldStorage(const std::filesystem::path& db_path);
    ~WorldStorage();
    rocksdb::DB* get_db();

    bool save_metadata(const std::string& data);

    std::optional<std::string> get_metadata();

private:
    std::unique_ptr<rocksdb::DB> m_db;

    static std::string make_metadata_key();
};
} // namespace cubed
