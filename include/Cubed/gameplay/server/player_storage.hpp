#pragma once
#include "Cubed/crypto/ed25519.hpp"
#include "Cubed/gameplay/item_stack.hpp"
#include "Cubed/tools/uuid.hpp"
#include "glm/ext/vector_float3.hpp"

#include <optional>
#include <span>
#include <string>
namespace cubed {
class WorldStorage;

struct PlayerStorageData {
    Uuid uuid{};
    glm::vec3 pos{0.0f, 255.0f, 0.0f};
    crypto::Ed25519PublicKey public_key;
    float yaw = 0.0f;
    float pitch = 0.0f;
    std::vector<StoredItemStack> inventory;
};

class PlayerStorage {
public:
    PlayerStorage(const PlayerStorage&) = default;
    PlayerStorage(PlayerStorage&&) = default;
    PlayerStorage& operator=(const PlayerStorage&) = delete;
    PlayerStorage& operator=(PlayerStorage&&) = delete;

    explicit PlayerStorage(WorldStorage& storage);
    ~PlayerStorage();

    std::optional<PlayerStorageData> load(const Uuid& uuid) const;

    bool save(const PlayerStorageData& data);

    bool remove(const Uuid& uuid);

    bool save_batch(std::span<const PlayerStorageData> players,
                    bool sync = false);

private:
    WorldStorage& m_storage;

    static std::string make_key(const Uuid& uuid);

    static std::string serialize(const PlayerStorageData& player);

    static std::optional<PlayerStorageData> deserialize(std::string_view data);
};
} // namespace cubed
