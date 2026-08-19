#pragma once
#include "Cubed/tools/compression.hpp"
#include "packet.pb.h" // IWYU pragma: keep

#include <concepts>
#include <cstdint>
#include <cstring>
#include <memory>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
namespace cubed {
constexpr size_t HEADER_LEN =
    sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t);
constexpr size_t PACKET_COMPRESSION_THRESHOLD = 100;
using Packet = std::shared_ptr<std::vector<uint8_t>>;
enum class CompressType : uint16_t {
    NONE = 0,
    ZSTD = 1,
};

inline CompressType get_compress_type(uint16_t id) {
    using enum CompressType;
    switch (id) {
    case std::to_underlying(NONE):
        return NONE;
    case std::to_underlying(ZSTD):
        return ZSTD;
    }
    throw std::runtime_error(std::format("Unknown CompressType {}", id));
}

struct PacketHeader {
    uint16_t cmd{};
    CompressType compress_type{}; // 0=none 1=zlib
    uint32_t uncompressed_size{};
    uint32_t compressed_size{};
};

enum class PacketEnum : uint16_t {
    LOGIN_REQ = 1001,
    LOGIN_RSP = 1002,
    LOGOUT_REQ = 1003,
    LOGOUT_RSP = 1004,
    LOGIN_CHALLENGE = 1005,
    LOGIN_PROOF = 1006,

    INVENTORY = 2001,
    C2S_PLAYER_INFO = 2002,
    PLAYER_INFO_RSP = 2003,
    PLAYER_WATER_SOUND = 2004,

    CHUNK_DATA_REQ = 3001,
    CHUNK_DATA_RSP = 3002,
    BLOCK_CHANGE_REQ = 3003,
    BLOCK_CHANGE_RSP = 3004,
    S2C_CLEAR_ALL_CHUNKS = 3005,
    UPDATE_TIME = 3006,
    S2C_ENTITY_CREATE = 3007,
    S2C_ENTITY_DESTROY = 3008,
    C2S_ENTITY_CREATE_REQ = 3009,
    C2S_ENTITY_DESTROY_REQ = 3010,
    S2C_ENTITY_UPDATE = 3011,
    S2C_ENTITY_UPDATE_BATCH = 3012,

    CHAT_MSG = 4001,
    VOICE_MSG = 4002,

    PING = 9001,
    PONG = 9002

};

template <typename> struct always_false : std::false_type {}; // NOLINT

template <typename T> constexpr uint16_t get_packet_id() {
    static_assert(always_false<T>::value, "Unknown Type");
    return 0;
}

template <> constexpr uint16_t get_packet_id<protocol::C2SLoginReq>() {
    return std::to_underlying(PacketEnum::LOGIN_REQ);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CLoginRsp>() {
    return std::to_underlying(PacketEnum::LOGIN_RSP);
}
template <> constexpr uint16_t get_packet_id<protocol::C2SLogoutReq>() {
    return std::to_underlying(PacketEnum::LOGOUT_REQ);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CLogoutRsp>() {
    return std::to_underlying(PacketEnum::LOGOUT_RSP);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CLoginChallenge>() {
    return std::to_underlying(PacketEnum::LOGIN_CHALLENGE);
}
template <> constexpr uint16_t get_packet_id<protocol::C2SLoginProof>() {
    return std::to_underlying(PacketEnum::LOGIN_PROOF);
}
template <> constexpr uint16_t get_packet_id<common::Inventory>() {
    return std::to_underlying(PacketEnum::INVENTORY);
}
template <> constexpr uint16_t get_packet_id<protocol::C2SPlayerInfo>() {
    return std::to_underlying(PacketEnum::C2S_PLAYER_INFO);
}
template <> constexpr uint16_t get_packet_id<protocol::PlayerInfoRsp>() {
    return std::to_underlying(PacketEnum::PLAYER_INFO_RSP);
}
template <> constexpr uint16_t get_packet_id<protocol::C2SChunkDataReq>() {
    return std::to_underlying(PacketEnum::CHUNK_DATA_REQ);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CChunkDataRsp>() {
    return std::to_underlying(PacketEnum::CHUNK_DATA_RSP);
}
template <> constexpr uint16_t get_packet_id<protocol::C2SBlockChangeReq>() {
    return std::to_underlying(PacketEnum::BLOCK_CHANGE_REQ);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CBlockChangeRsp>() {
    return std::to_underlying(PacketEnum::BLOCK_CHANGE_RSP);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CClearAllChunks>() {
    return std::to_underlying(PacketEnum::S2C_CLEAR_ALL_CHUNKS);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CEntityCreate>() {
    return std::to_underlying(PacketEnum::S2C_ENTITY_CREATE);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CEntityDestroy>() {
    return std::to_underlying(PacketEnum::S2C_ENTITY_DESTROY);
}
template <> constexpr uint16_t get_packet_id<protocol::C2SEntityCreateReq>() {
    return std::to_underlying(PacketEnum::C2S_ENTITY_CREATE_REQ);
}
template <> constexpr uint16_t get_packet_id<protocol::C2SEntityDestroyReq>() {
    return std::to_underlying(PacketEnum::C2S_ENTITY_DESTROY_REQ);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CUpdateTime>() {
    return std::to_underlying(PacketEnum::UPDATE_TIME);
}
template <> constexpr uint16_t get_packet_id<protocol::Ping>() {
    return std::to_underlying(PacketEnum::PING);
}
template <> constexpr uint16_t get_packet_id<protocol::Pong>() {
    return std::to_underlying(PacketEnum::PONG);
}
template <> constexpr uint16_t get_packet_id<protocol::PlayerWaterSound>() {
    return std::to_underlying(PacketEnum::PLAYER_WATER_SOUND);
}
template <> constexpr uint16_t get_packet_id<protocol::ChatMsg>() {
    return std::to_underlying(PacketEnum::CHAT_MSG);
}
template <> constexpr uint16_t get_packet_id<protocol::VoiceMsg>() {
    return std::to_underlying(PacketEnum::VOICE_MSG);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CEntityUpdate>() {
    return std::to_underlying(PacketEnum::S2C_ENTITY_UPDATE);
}
template <> constexpr uint16_t get_packet_id<protocol::S2CEntityUpdateBatch>() {
    return std::to_underlying(PacketEnum::S2C_ENTITY_UPDATE_BATCH);
}

template <typename T>
    requires std::derived_from<T, google::protobuf::Message>
Packet make_packet(const T& msg) {
    PacketHeader header{};
    header.cmd = get_packet_id<T>();
    uint32_t raw_size = static_cast<uint32_t>(msg.ByteSizeLong());
    std::vector<uint8_t> raw(raw_size);

    if (!msg.SerializeToArray(raw.data(), raw_size)) {
        return {};
    }
    std::vector<uint8_t> payload;
    if (raw_size >= PACKET_COMPRESSION_THRESHOLD) {
        std::vector<uint8_t> compressed = compress_data(raw);
        if (compressed.size() < raw.size()) {
            payload = std::move(compressed);
            header.compress_type = CompressType::ZSTD;
        } else {
            payload = std::move(raw);
            header.compress_type = CompressType::NONE;
        }
    } else {
        payload = std::move(raw);
        header.compress_type = CompressType::NONE;
    }
    header.uncompressed_size = raw_size;
    header.compressed_size = static_cast<uint32_t>(payload.size());

    auto packet =
        std::make_shared<std::vector<uint8_t>>(HEADER_LEN + payload.size());

    uint16_t cmd_net = htons(header.cmd);
    uint16_t compress_type_net =
        htons(std::to_underlying(header.compress_type));
    uint32_t uncompressed_size_net = htonl(header.uncompressed_size);
    uint32_t compressed_size_net = htonl(header.compressed_size);

    std::memcpy(packet->data(), &cmd_net, sizeof(cmd_net));

    std::memcpy(packet->data() + 2, &compress_type_net,
                sizeof(compress_type_net));
    std::memcpy(packet->data() + 4, &uncompressed_size_net,
                sizeof(uncompressed_size_net));
    std::memcpy(packet->data() + 8, &compressed_size_net,
                sizeof(compressed_size_net));
    std::memcpy(packet->data() + HEADER_LEN, payload.data(), payload.size());

    return packet;
}

template <typename T>
    requires std::derived_from<T, google::protobuf::Message>
Packet make_packet(const T* msg) {
    return make_packet(*msg);
}

inline PacketHeader decode_packet_header(std::span<const uint8_t> header) {
    if (header.size() < HEADER_LEN)
        throw std::runtime_error("Invalid header");
    uint16_t cmd_net;
    uint16_t compress_type_net;
    uint32_t uncompressed_size_net;
    uint32_t compressed_size_net;
    std::memcpy(&cmd_net, header.data(), sizeof(cmd_net));
    std::memcpy(&compress_type_net, header.data() + 2,
                sizeof(compress_type_net));
    std::memcpy(&uncompressed_size_net, header.data() + 4,
                sizeof(uncompressed_size_net));
    std::memcpy(&compressed_size_net, header.data() + 8,
                sizeof(compressed_size_net));

    return {ntohs(cmd_net), get_compress_type(ntohs(compress_type_net)),
            ntohl(uncompressed_size_net), ntohl(compressed_size_net)};
}
template <typename T>
    requires std::derived_from<T, google::protobuf::Message>
bool decode_packet(T& message, std::span<const uint8_t> data,
                   const PacketHeader& header) {
    if (data.size() != header.compressed_size) {
        return false;
    }

    if (header.compress_type == CompressType::NONE &&
        header.uncompressed_size != header.compressed_size) {
        return false;
    }

    switch (header.compress_type) {
    case CompressType::NONE: {
        return message.ParseFromArray(
            data.data(), static_cast<int>(header.uncompressed_size));
    }
    case CompressType::ZSTD: {
        auto raw = decompress_data(data, header.uncompressed_size);
        return message.ParseFromArray(raw.data(), static_cast<int>(raw.size()));
    }
    default:
        return false;
    }
}

} // namespace cubed
