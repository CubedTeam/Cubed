#pragma once
#include "Cubed/gameplay/server/player_storage.hpp"
#include "Cubed/gameplay/server/server_player.hpp"
#include "Cubed/tools/uuid.hpp"

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>
namespace Cubed {
class ServerWorld;
class ServerPlayerManager {
public:
    using PlayerPtr = std::shared_ptr<ServerPlayer>;
    using PlayerMap = std::unordered_map<Uuid, PlayerPtr>;
    using PlayerMapPtr = std::shared_ptr<const PlayerMap>;
    ServerPlayerManager(ServerWorld& world);

    ~ServerPlayerManager();

    void stop();
    void init();

    void update();

    bool add(PlayerPtr player);
    PlayerPtr remove(const Uuid& uuid, const PlayerPtr& expected_player);

    [[nodiscard]] PlayerPtr find(const Uuid& uuid) const;
    [[nodiscard]] bool contains(const Uuid& uuid) const;
    [[nodiscard]] std::size_t sum() const;
    [[nodiscard]] bool empty() const;

    [[nodiscard]] int get_task_id(const Uuid& uuid) const;
    [[nodiscard]] std::optional<glm::vec3> get_position(const Uuid& uuid) const;

    [[nodiscard]] std::shared_ptr<Session> get_session(const Uuid& uuid) const;

    [[nodiscard]] ServerPlayerManager::PlayerMapPtr snapshot() const;

    std::vector<std::shared_ptr<Session>> get_all_session() const;

    PlayerStorage* get_storage();

    void save_all();

    PlayerStorageData build_data(const ServerPlayer& player);

private:
    ServerWorld& m_world;
    std::unique_ptr<PlayerStorage> m_storage;
    std::mutex m_write_mutex;

    // key = uuid
    std::atomic<std::shared_ptr<const PlayerMap>> m_players;
};
} // namespace Cubed