#pragma once
#include "packet.pb.h"

#include <netinet/in.h>
#include <type_traits>
namespace Cubed {
constexpr int HEADER_LEN = 8;
using Packet = std::shared_ptr<std::vector<uint8_t>>;
template <typename> struct always_false : std::false_type {}; // NOLINT
template <typename T> constexpr uint16_t get_packet_id() {

    using std::is_same_v;
    using U = std::decay_t<T>;
    if constexpr (is_same_v<U, LoginReq>) {
        return 1001;
    } else if constexpr (is_same_v<U, LoginRsp>) {
        return 1002;
    } else if constexpr (is_same_v<U, PlayerInfo>) {
        return 2001;
    } else if constexpr (is_same_v<U, PlayerPos>) {
        return 2002;
    } else if constexpr (is_same_v<U, ChunkData>) {
        return 3001;
    } else if constexpr (is_same_v<U, Ping>) {
        return 9001;
    } else if (is_same_v<U, Pong>) {
        return 9002;
    } else {
        static_assert(always_false<U>::value, "Unkonw Type");
    }
}

template <typename T> Packet make_packet(const T& msg) {
    std::string body;

    if (!msg.SerializeToString(&body)) {
        return {};
    }

    uint16_t cmd = get_packet_id<T>();

    uint32_t total_len = HEADER_LEN + body.size();

    auto packet = std::make_shared<std::vector<uint8_t>>(total_len);

    uint32_t total_len_net = htonl(total_len);
    uint16_t cmd_net = htons(cmd);

    std::memcpy(packet->data(), &total_len_net, sizeof(total_len_net));

    std::memcpy(packet->data() + sizeof(total_len_net), &cmd_net,
                sizeof(cmd_net));

    std::memcpy(packet->data() + HEADER_LEN, body.data(), body.size());

    return packet;
}

} // namespace Cubed
