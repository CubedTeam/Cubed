#pragma once
#include <filesystem>

namespace rocksdb {
class DB;
}
namespace Cubed {
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

private:
    std::unique_ptr<rocksdb::DB> m_db;
};
} // namespace Cubed