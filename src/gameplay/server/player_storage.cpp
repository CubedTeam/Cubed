#include "Cubed/gameplay/server/player_storage.hpp"

#include "Cubed/gameplay/server/world_storage.hpp"
#include "Cubed/tools/json_utils.hpp"
#include "Cubed/tools/log.hpp"

#include <rapidjson/document.h>
#include <rocksdb/db.h>
using namespace rapidjson;
namespace Cubed {
PlayerStorage::PlayerStorage(WorldStorage& storage) : m_storage(storage) {}
PlayerStorage::~PlayerStorage() {}

std::optional<PlayerStorageData> PlayerStorage::load(const Uuid& uuid) const {
    std::string value;
    auto status =
        m_storage.get_db()->Get(rocksdb::ReadOptions{}, make_key(uuid), &value);

    if (status.IsNotFound()) {
        return std::nullopt;
    }

    if (!status.ok()) {
        Logger::error("Failed to load player {} : {}", uuid.to_string(),
                      status.ToString());
        return std::nullopt;
    }

    auto player = deserialize(value);

    if (!player) {
        Logger::error("Failed to deserialize entity {}", uuid.to_string());
    }

    return player;
}

bool PlayerStorage::save(const PlayerStorageData& data) {
    auto value = serialize(data);
    if (value.empty()) {
        return false;
    }

    auto status = m_storage.get_db()->Put(rocksdb::WriteOptions{},
                                          make_key(data.uuid), value);

    if (!status.ok()) {
        Logger::error("Failed to save entity {}: {}", data.uuid.to_string(),
                      status.ToString());
        return false;
    }
    return true;
}

bool PlayerStorage::save_batch(std::span<const PlayerStorageData> players,
                               bool sync) {
    if (players.empty()) {
        return true;
    }

    rocksdb::WriteBatch batch;
    bool error = false;
    for (const auto& player : players) {
        auto value = serialize(player);

        if (value.empty()) {
            Logger::error("Failed to serialize entity {}",
                          player.uuid.to_string());
            error = true;
            continue;
        }

        batch.Put(make_key(player.uuid), value);
    }

    rocksdb::WriteOptions options;
    options.sync = sync;

    auto status = m_storage.get_db()->Write(options, &batch);

    if (!status.ok()) {
        Logger::error("Failed to save {} entities: {}", players.size(),
                      status.ToString());
        return false;
    }

    return !error;
}

bool PlayerStorage::remove(const Uuid& uuid) {
    rocksdb::WriteOptions write_options;

    auto status = m_storage.get_db()->Delete(write_options, make_key(uuid));

    if (!status.ok()) {
        Logger::error("Failed to deleta entity {}: {}", uuid.to_string(),
                      status.ToString());
        return false;
    }

    return true;
}

std::string PlayerStorage::make_key(const Uuid& uuid) {
    return std::format("player:{}", uuid.to_string());
}

std::string PlayerStorage::serialize(const PlayerStorageData& player) {
    Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();
    doc.AddMember("uuid",
                  rapidjson::Value(player.uuid.to_string().c_str(), allocator),
                  allocator);
    doc.AddMember("version", WorldStorage::VERSION, allocator);
    auto j_pos = Tools::vec3_to_json(player.pos, allocator);
    doc.AddMember("position", j_pos, allocator);
    std::string pk = Crypto::Ed25519::to_hex(player.public_key.data.data(),
                                             player.public_key.data.size());
    doc.AddMember("public_key", rapidjson::Value(pk.c_str(), allocator),
                  allocator);
    doc.AddMember("yaw", player.yaw, allocator);
    doc.AddMember("pitch", player.pitch, allocator);
    return Tools::to_json_string(doc);
}

std::optional<PlayerStorageData>
PlayerStorage::deserialize(std::string_view data) {

    Document doc;
    if (!Tools::parse_json_from_string(doc, data)) {
        Logger::error("Can't parse Player Storage Data");
        return std::nullopt;
    }

    uint32_t version = 0;
    if (Tools::get_json_value(doc, "version", version)) {
        if (version > WorldStorage::VERSION) {
            Logger::error("Unsupported player version: {}", version);
            return std::nullopt;
        }
    }
    PlayerStorageData player;

    std::string uuid;
    if (Tools::get_json_value(doc, "uuid", uuid)) {
        auto u = Uuid::from_string(uuid);
        if (!u) {
            Logger::error("Parse uuid from player string fail");
            return std::nullopt;
        }
        player.uuid = *u;
    } else {
        return std::nullopt;
    }
    std::string pk;
    if (Tools::get_json_value(doc, "public_key", pk)) {
        if (!Crypto::Ed25519::from_hex(pk, player.public_key.data.data(),
                                       player.public_key.data.size())) {
            Logger::error("Parse player public key from hex fail.");
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }

    if (Crypto::Ed25519::uuid_from_public_key(player.public_key) !=
        player.uuid) {
        Logger::error("Player {} pk != uuid", player.uuid.to_string());
        return std::nullopt;
    }

    if (!Tools::get_json_value(doc, "position", player.pos)) {
        Logger::error("Parse player position fail");
        return std::nullopt;
    }

    if (!Tools::get_json_value(doc, "yaw", player.yaw)) {
        Logger::error("Parse player yaw fail");
    }
    if (!Tools::get_json_value(doc, "pitch", player.pitch)) {
        Logger::error("Parse player pitch fail");
    }

    return player;
}

} // namespace Cubed