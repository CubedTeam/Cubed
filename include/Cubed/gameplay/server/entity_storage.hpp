#pragma once
#include "Cubed/gameplay/ecs/entity.hpp"
#include "glm/ext/vector_float3.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
namespace rocksdb {
class DB;
}

namespace cubed {
class ServerEntityManager;
class WorldStorage;
struct EntityStorageData {
    EntityID id = 0;
    std::string name;
    glm::vec3 pos{0.0f};
    glm::vec3 dir{0.0f};
};

class EntityStorage {

public:
    explicit EntityStorage(WorldStorage& storage);

    ~EntityStorage();

    EntityStorage(const EntityStorage&) = delete;
    EntityStorage(EntityStorage&&) = delete;
    EntityStorage& operator=(const EntityStorage&) = delete;
    EntityStorage& operator=(EntityStorage&&) = delete;

    std::optional<EntityStorageData> load(EntityID id) const;

    bool save(const EntityStorageData& entity);

    bool save_batch(std::span<const EntityStorageData> entities,
                    bool sync = false);

    bool remove(EntityID id);
    bool remove_batch(std::span<const EntityID> entities);
    // std::size_t size() const;

    [[nodiscard]]
    std::vector<EntityStorageData> load_all();

private:
    WorldStorage& m_storage;

    static std::string make_key(EntityID id);
    static std::string serialize(const EntityStorageData& entity);

    static std::optional<EntityStorageData> deserialize(std::string_view data);
};
} // namespace cubed
