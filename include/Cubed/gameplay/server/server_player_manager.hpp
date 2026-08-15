#pragma once
#include "Cubed/gameplay/server/player_storage.hpp"
#include "Cubed/gameplay/server/server_player.hpp"
#include "Cubed/tools/uuid.hpp"

#include <atomic>
#include <mutex>
#include <string>

namespace Cubed {
class ServerWorld;
class ServerPlayerManager {
public:
    using PlayerPtr = std::shared_ptr<ServerPlayer>;
    using PlayerMap =
        std::unordered_map<std::string, PlayerPtr, TransparentStringHash,
                           std::equal_to<>>;
    using PlayerMapPtr = std::shared_ptr<const PlayerMap>;
    ServerPlayerManager(ServerWorld& world);

    ~ServerPlayerManager();

    void stop();
    void init();

    bool add(PlayerPtr player);
    PlayerPtr remove(std::string_view uuid);

    [[nodiscard]] PlayerPtr find(std::string_view uuid) const;
    [[nodiscard]] bool contains(std::string_view uuid) const;
    [[nodiscard]] std::size_t sum() const;
    [[nodiscard]] bool empty() const;

    [[nodiscard]] int get_task_id(std::string_view uuid) const;
    [[nodiscard]] std::optional<glm::vec3>
    get_position(std::string_view uuid) const;

    [[nodiscard]] std::shared_ptr<Session>
    get_session(std::string_view uuid) const;

    [[nodiscard]] ServerPlayerManager::PlayerMapPtr snapshot() const;

    std::vector<std::shared_ptr<Session>> get_all_session() const;

    PlayerStorage* get_storage();

    void save_all();

private:
    ServerWorld& m_world;
    std::unique_ptr<PlayerStorage> m_storage;
    std::mutex m_write_mutex;

    // key = uuid
    std::atomic<std::shared_ptr<const PlayerMap>> m_players;
};
} // namespace Cubed