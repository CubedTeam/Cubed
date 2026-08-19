#pragma once
#include "Cubed/gameplay/hitbox.hpp"

#include <tbb/concurrent_hash_map.h>
namespace cubed {
class HitboxManager {
public:
    struct Handle {
        Hitbox box{};
        HitboxID id = 0;
    };
    HitboxManager();
    ~HitboxManager();
    static HitboxManager& instance();

    [[nodiscard]]
    Handle get_hitbox(const std::string& key);
    [[nodiscard]]
    Handle get_hitbox(HitboxID id);
    [[nodiscard]]
    static Handle hitbox(const std::string& name);
    [[nodiscard]]
    static Handle hitbox(HitboxID id);
    HitboxID get_hitbox_id(const std::string& name);
    std::string get_hitbox_name(HitboxID id);

private:
    using HitboxMap = tbb::concurrent_hash_map<HitboxID, Hitbox>;
    using IDMap = tbb::concurrent_hash_map<std::string, HitboxID>;
    using NameMap = tbb::concurrent_hash_map<HitboxID, std::string>;

    HitboxID m_next = 0;
    IDMap m_id_map;
    NameMap m_name_map;
    HitboxMap m_hitboxes;
    Handle load(std::string_view name);
};
} // namespace cubed
