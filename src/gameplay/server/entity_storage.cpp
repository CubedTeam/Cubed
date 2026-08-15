#include "Cubed/gameplay/server/entity_storage.hpp"

#include "Cubed/gameplay/server/world_storage.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/proto_utils.hpp"
#include "save/stored_entity.pb.h"

#include <rocksdb/db.h>

using namespace google::protobuf;

namespace Cubed {
EntityStorage::EntityStorage(WorldStorage& storage) : m_storage(storage) {}

EntityStorage::~EntityStorage() {}

std::optional<EntityStorageData> EntityStorage::load(EntityID id) const {
    std::string value;
    auto status =
        m_storage.get_db()->Get(rocksdb::ReadOptions{}, make_key(id), &value);

    if (status.IsNotFound()) {
        return std::nullopt;
    }

    if (!status.ok()) {
        Logger::error("Failed to load entity {} : {}", id, status.ToString());
        return std::nullopt;
    }

    auto entity = deserialize(value);

    if (!entity) {
        Logger::error("Failed to deserialize entity {}", id);
    }

    return entity;
}

bool EntityStorage::save(const EntityStorageData& entity) {
    auto value = serialize(entity);
    if (value.empty()) {
        return false;
    }

    auto status = m_storage.get_db()->Put(rocksdb::WriteOptions{},
                                          make_key(entity.id), value);

    if (!status.ok()) {
        Logger::error("Failed to save entity {}: {}", entity.id,
                      status.ToString());
        return false;
    }
    return true;
}

bool EntityStorage::save_batch(std::span<const EntityStorageData> entities,
                               bool sync) {
    if (entities.empty()) {
        return true;
    }

    rocksdb::WriteBatch batch;
    bool error = false;
    for (const auto& entity : entities) {
        auto value = serialize(entity);

        if (value.empty()) {
            Logger::error("Failed to serialize entity {}", entity.id);
            error = true;
            continue;
        }

        batch.Put(make_key(entity.id), value);
    }

    rocksdb::WriteOptions options;
    options.sync = sync;

    auto status = m_storage.get_db()->Write(options, &batch);

    if (!status.ok()) {
        Logger::error("Failed to save {} entities: {}", entities.size(),
                      status.ToString());
        return false;
    }

    return !error;
}

bool EntityStorage::remove(EntityID id) {
    rocksdb::WriteOptions write_options;

    auto status = m_storage.get_db()->Delete(write_options, make_key(id));

    if (!status.ok()) {
        Logger::error("Failed to deleta entity {}: {}", id, status.ToString());
        return false;
    }

    return true;
}

bool EntityStorage::remove_batch(std::span<const EntityID> entities) {
    rocksdb::WriteBatch batch;

    for (EntityID id : entities) {
        batch.Delete(make_key(id));
    }

    rocksdb::WriteOptions options;

    auto status = m_storage.get_db()->Write(options, &batch);

    if (!status.ok()) {
        Logger::error("Failed to deleta entities size {}: {}", entities.size(),
                      status.ToString());
        return false;
    }

    return true;
}

std::vector<EntityStorageData> EntityStorage::load_all() {
    constexpr std::string_view PREFIX = "entity:";
    std::vector<EntityStorageData> datas;
    std::unique_ptr<rocksdb::Iterator> it{
        m_storage.get_db()->NewIterator(rocksdb::ReadOptions{})};

    for (it->Seek(PREFIX); it->Valid(); it->Next()) {
        const std::string_view KEY{it->key().data(), it->key().size()};

        if (!KEY.starts_with(PREFIX)) {
            break;
        }

        auto entity = deserialize(it->value().ToStringView());
        if (!entity) {
            Logger::error("Can't create entity {}", KEY);
            continue;
        }
        // entity manager add entity;
        datas.emplace_back(std::move(*entity));
    }

    return datas;
}

std::string EntityStorage::make_key(EntityID id) {
    return std::format("entity:{}", id);
}

std::string EntityStorage::serialize(const EntityStorageData& entity) {

    Arena arena;

    auto* msg = Arena::Create<StoredEntity>(&arena);

    msg->set_id(entity.id);
    msg->set_name(entity.name);
    msg->set_version(WorldStorage::VERSION);

    Tools::set_proto_vec3(msg->mutable_pos(), entity.pos);
    Tools::set_proto_vec3(msg->mutable_dir(), entity.dir);

    uint32_t raw_size = static_cast<uint32_t>(msg->ByteSizeLong());
    std::string raw;
    raw.resize(raw_size);

    if (!msg->SerializeToArray(raw.data(), raw_size)) {
        return {};
    }

    return raw;
}

std::optional<EntityStorageData>
EntityStorage::deserialize(std::string_view data) {

    Arena arena;

    auto* msg = Arena::Create<StoredEntity>(&arena);
    if (!msg->ParseFromArray(data.data(), data.size())) {
        return std::nullopt;
    }

    if (msg->version() > WorldStorage::VERSION) {
        Logger::error("Unsupported entity version: {}", msg->version());
        return std::nullopt;
    }

    EntityStorageData d;

    d.id = msg->id();
    d.name = msg->name();

    d.pos = Tools::get_proto_vec3(msg->pos());
    d.dir = Tools::get_proto_vec3(msg->dir());

    return d;
}

} // namespace Cubed