#pragma once
#include "Cubed/AABB.hpp"

#include <tbb/concurrent_hash_map.h>
namespace Cubed {
class HitboxManager {
public:
    HitboxManager();
    ~HitboxManager();
    static HitboxManager& instance();

    AABB get_aabb(const std::string& key);

    static AABB aabb(const std::string& key);

private:
    using HitBoxMap = tbb::concurrent_hash_map<std::string, AABB>;
    using cacc = HitBoxMap::const_accessor;
    using acc = HitBoxMap::accessor;
    HitBoxMap m_hitboxes;

    AABB load(const std::string& path);
};
} // namespace Cubed