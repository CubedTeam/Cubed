#pragma once
#include "Cubed/tools/resource_location.hpp"

#include <optional>
#include <string_view>
#include <tbb/concurrent_hash_map.h>
namespace Cubed {

struct CreatureSound {
    std::optional<ResourceLocation> call;
};

struct CreatureData {
    ResourceLocation name{};
    std::optional<ResourceLocation> model{};
    std::optional<ResourceLocation> animation{};
    std::optional<ResourceLocation> collision{};
    CreatureSound sound;
};

class CreatureManager {
public:
    CreatureManager();
    ~CreatureManager();

    static CreatureManager& instance();
    void init();

    CreatureData get_creature_data(std::string_view name) const;
    CreatureData get_creature_data(const ResourceLocation& location) const;

    static CreatureData data(std::string_view name);
    static CreatureData data(const ResourceLocation& location);

private:
    using CreatureMap = tbb::concurrent_hash_map<ResourceLocation, CreatureData,
                                                 ResourceLocation::Hash>;

    using cacc = CreatureMap::const_accessor;
    using acc = CreatureMap::accessor;
    static inline const CreatureData EMPTY;
    CreatureMap m_creature_map;
};
} // namespace Cubed