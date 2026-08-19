#include "Cubed/gameplay/server/server_player_manager.hpp"

#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/gameplay/server/session.hpp"
namespace cubed {
ServerPlayerManager::ServerPlayerManager(ServerWorld& world)
    : m_world(world), m_players(std::make_shared<PlayerMap>()) {}
ServerPlayerManager::~ServerPlayerManager() {}

void ServerPlayerManager::stop() { save_all(); }

void ServerPlayerManager::init() {
    m_storage = std::make_unique<PlayerStorage>(*m_world.world_storage());
}

void ServerPlayerManager::update() {
    auto players_map = snapshot();
    if (!players_map) {
        return;
    }
    std::vector<PlayerPtr> players;
    players.reserve(players_map->size());
    for (const auto& [_, p] : *players_map) {
        players.emplace_back(p);
    }

    auto pool = m_world.get_compute_pool();
    if (pool) {
        parallel_do(*pool, players.begin(), players.end(), pool->thread_sum(),
                    [](PlayerPtr& player) {
                        if (player) {
                            player->update();
                        }
                    });
    } else {
        for (auto& player : players) {
            player->update();
        }
    }
}

bool ServerPlayerManager::add(PlayerPtr player) {
    if (!player) {
        return false;
    }

    auto uuid = player->get_uuid();

    std::lock_guard lock(m_write_mutex);

    auto old = m_players.load();
    auto next = std::make_shared<PlayerMap>(*old);
    auto [it, insert] = next->try_emplace(uuid, std::move(player));
    if (insert) {
        m_players.store(next);
        auto data = m_storage->load(it->second->get_uuid());
        if (data) {
            it->second->update_pos(data->pos.x, data->pos.y, data->pos.z);
            it->second->set_yaw(data->yaw);
            it->second->set_pitch(data->pitch);

            for (const auto& stack : data->inventory) {
                ItemStack s;
                s.item = stack.item_id;
                s.count = stack.count;
                it->second->add(s, stack.position);
            }

        } else {
            data = PlayerStorageData{};
            data->pos = it->second->get_pos();
            data->yaw = it->second->yaw();
            data->pitch = it->second->pitch();
        }
        data->public_key = it->second->get_session()->public_key();
        data->uuid = it->second->get_uuid();

        m_storage->save(*data);
    }

    return insert;
}

ServerPlayerManager::PlayerPtr
ServerPlayerManager::remove(const Uuid& uuid,
                            const PlayerPtr& expected_player) {
    if (!expected_player) {
        return {};
    }

    std::lock_guard lock(m_write_mutex);

    auto old = m_players.load();
    auto old_it = old->find(uuid);
    if (old_it == old->end() || old_it->second != expected_player) {
        return {};
    }

    auto next = std::make_shared<PlayerMap>(*old);
    auto it = next->find(uuid);
    auto player = it->second;
    next->erase(it);

    m_players.store(next);

    PlayerStorageData data = build_data(*player);
    m_storage->save(data);

    return player;
}

ServerPlayerManager::PlayerPtr
ServerPlayerManager::find(const Uuid& uuid) const {
    auto players = m_players.load();
    auto it = players->find(uuid);
    if (it == players->end()) {
        return {};
    }
    return it->second;
}

bool ServerPlayerManager::contains(const Uuid& uuid) const {
    auto players = m_players.load();
    return players->contains(uuid);
}

std::size_t ServerPlayerManager::sum() const { return snapshot()->size(); }

bool ServerPlayerManager::empty() const {

    auto players = m_players.load();
    return players->empty();
}

std::optional<glm::vec3>
ServerPlayerManager::get_position(const Uuid& uuid) const {

    auto player = find(uuid);
    if (!player) {
        return std::nullopt;
    }

    return player->get_pos();
}

std::shared_ptr<Session>
ServerPlayerManager::get_session(const Uuid& uuid) const {
    auto player = find(uuid);
    return player ? player->get_session() : nullptr;
}

ServerPlayerManager::PlayerMapPtr ServerPlayerManager::snapshot() const {
    return m_players.load();
}

int ServerPlayerManager::get_task_id(const Uuid& uuid) const {
    auto players = snapshot();
    auto it = players->find(uuid);

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
        PlayerStorageData data = build_data(*player);

        datas.emplace_back(std::move(data));
    }

    m_storage->save_batch(datas);
}

PlayerStorageData ServerPlayerManager::build_data(const ServerPlayer& player) {
    PlayerStorageData data;
    data.pos = player.get_pos();
    data.public_key = player.get_session()->public_key();
    data.uuid = player.get_uuid();
    data.yaw = player.yaw();
    data.pitch = player.pitch();
    auto inventory = player.inventory();

    for (size_t i = 0; i < inventory.size(); ++i) {
        if (inventory[i]) {
            StoredItemStack stack;
            stack.item_id = inventory[i]->item;
            stack.position = i;
            stack.count = inventory[i]->count;
            data.inventory.push_back(std::move(stack));
        }
    }

    return data;
}

PlayerStorage* ServerPlayerManager::get_storage() { return m_storage.get(); }

} // namespace cubed
