#pragma once

#include "Cubed/gameplay/item.hpp"

#include <filesystem>
#include <rapidjson/document.h>
#include <string_view>
#include <tbb/concurrent_hash_map.h>
namespace Cubed {
class ItemManager {
public:
    ItemManager();
    void init();
    static ItemManager& instance();

    static ItemData get(std::string_view key);
    static ItemData get(ItemID id);

    static ItemID size();

    ItemData get_item_data(std::string_view key) const;
    ItemData get_item_data(ItemID id) const;

private:
    using ItemMap = tbb::concurrent_hash_map<ItemID, ItemData>;
    using acc = ItemMap::accessor;
    using cacc = ItemMap::const_accessor;

    using IDMap = tbb::concurrent_hash_map<std::string, ItemID>;
    using BlockToIDMap = tbb::concurrent_hash_map<BlockType, ItemID>;
    void add(const std::filesystem::path& path,
             const rapidjson::Value& registry);

    ItemMap m_map;

    IDMap m_id_map;
    BlockToIDMap m_block_to_id_map;

    static inline const ItemData EMPTY;
};
} // namespace Cubed