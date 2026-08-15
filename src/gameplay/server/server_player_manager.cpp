#include "Cubed/gameplay/server/server_player_manager.hpp"

#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/gameplay/server/session.hpp"
namespace Cubed {
ServerPlayerManager::ServerPlayerManager(ServerWorld& world)
    : m_world(world), m_players(std::make_shared<PlayerMap>()) {}
ServerPlayerManager::~ServerPlayerManager() {}

void ServerPlayerManager::stop() { save_all(); }

void ServerPlayerManager::init() {
    m_storage = std::make_unique<PlayerStorage>(*m_world.world_storage());
}

bool ServerPlayerManager::add(PlayerPtr player) {
    if (!player) {
        return false;
    }

    const auto& uuid = player->get_uuid_string();

    std::lock_guard lock(m_write_mutex);

    auto old = m_players.load();
    auto next = std::make_shared<PlayerMap>(*old);
    auto [it, insert] = next->try_emplace(uuid, std::move(player));
    if (insert) {
        m_players.store(next);
        auto data = m_storage->load(player->get_uuid());
        if (data) {
            player->update_pos(data->pos.x, data->pos.y, data->pos.z);
        } else {
            data = PlayerStorageData{};
            data->pos = player->get_pos();
        }
        data->public_key = it->second->get_session()->public_key();
        data->uuid = it->second->get_uuid();
        m_storage->save(*data);
    }

    return insert;
}

ServerPlayerManager::PlayerPtr
ServerPlayerManager::remove(std::string_view uuid) {
    std::lock_guard lock(m_write_mutex);
    auto old = m_players.load();
    if (!old->contains(uuid)) {
        return {};
    }

    auto next = std::make_shared<PlayerMap>(*old);

    auto it = next->find(uuid);
    auto player = it->second;
    next->erase(it);

    m_players.store(next);

    PlayerStorageData data;
    data.pos = player->get_pos();
    data.public_key = player->get_session()->public_key();
    data.uuid = player->get_uuid();
    m_storage->save(data);

    return player;
}

ServerPlayerManager::PlayerPtr
ServerPlayerManager::find(std::string_view uuid) const {
    auto players = m_players.load();
    auto it = players->find(uuid);
    if (it == players->end()) {
        return {};
    }
    return it->second;
}

bool ServerPlayerManager::contains(std::string_view uuid) const {
    auto players = m_players.load();
    return players->contains(uuid);
}

std::size_t ServerPlayerManager::sum() const { return snapshot()->size(); }

bool ServerPlayerManager::empty() const {

    auto players = m_players.load();
    return players->empty();
}

std::optional<glm::vec3>
ServerPlayerManager::get_position(std::string_view uuid) const {

    auto player = find(uuid);
    if (!player) {
        return std::nullopt;
    }

    return player->get_pos();
}

std::shared_ptr<Session>
ServerPlayerManager::get_session(std::string_view uuid) const {
    auto player = find(uuid);
    return player ? player->get_session() : nullptr;
}

ServerPlayerManager::PlayerMapPtr ServerPlayerManager::snapshot() const {
    return m_players.load();
}

int ServerPlayerManager::get_task_id(std::string_view id) const {
    auto players = snapshot();
    auto it = players->find(id);

    return it == players->end() ? -1 : it->second->task_id();
}

std::vector<std::shared_ptr<Session>>
ServerPlayerManager::get_all_session() const {
    auto players = snapshot();
    if (!players) {
        Logger::error("Can't get players map");
        return {};
    }

    std::vector<std::shared_ptr<Session>> sessions;

    for (auto [_, p] : *players) {
        sessions.emplace_back(p->get_session());
    }

    return sessions;
}

void ServerPlayerManager::save_all() {
    auto players = snapshot();
    std::vector<PlayerStorageData> datas;
    for (auto& [_, player] : *players) {
        PlayerStorageData data;
        data.pos = player->get_pos();
        data.public_key = player->get_session()->public_key();
        data.uuid = player->get_uuid();
        datas.emplace_back(std::move(data));
    }

    m_storage->save_batch(datas);
}

PlayerStorage* ServerPlayerManager::get_storage() { return m_storage.get(); }

} // namespace Cubed