#pragma once

#include "Cubed/gameplay/block.hpp"

#include <tbb/concurrent_hash_map.h>
namespace cubed {
class BlockManager {

public:
    static void init();
    static unsigned sums();
    static unsigned cross_plane_sum();
    static const ResourceLocation& name_form_id(BlockType id);
    static bool is_gas(BlockType id);
    static bool is_liquid(BlockType id);

    static bool is_water(BlockType id);

    static bool is_cross_plane(BlockType id);
    static bool is_transparent(BlockType id);
    static bool is_passable(BlockType id);

    static bool is_discard(BlockType id);
    static bool is_blend(BlockType id);
    static bool is_transitional(BlockType id);
    static float roughness(BlockType id);
    static BlockType cross_plane_index(BlockType id);

    static BlockType id_from_name(std::string_view name);
    static BlockType id_from_name(const ResourceLocation& name);

    static BlockData data(std::string_view name);
    static BlockData data(const ResourceLocation& location);
    static BlockData data(BlockType type);

private:
    using BlockMap = tbb::concurrent_hash_map<BlockType, BlockData>;
    using acc = BlockMap::accessor;
    using cacc = BlockMap::const_accessor;
    using IDMap = tbb::concurrent_hash_map<ResourceLocation, BlockType,
                                           ResourceLocation::Hash>;
    using CrossPlaneMap = tbb::concurrent_hash_map<BlockType, BlockType>;

    static inline const BlockData EMPTY;
    static inline BlockType m_water = 7;
    static inline BlockMap m_datas;
    static inline IDMap m_id_map;
    static inline bool is_init = false;
    static inline CrossPlaneMap m_cross_plane_map;
    static void set_up_cross_plane_map(
        const std::vector<std::pair<bool, BlockType>>& types);
};
} // namespace cubed
