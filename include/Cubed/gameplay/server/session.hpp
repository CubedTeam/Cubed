#pragma once

#include "Cubed/crypto/ed25519.hpp"
#include "Cubed/gameplay/packet.hpp"

#include <asio.hpp>
#include <memory>
#include <queue>
#include <string>
namespace cubed {

using asio::ip::tcp;
class ServerWorld;
class Session : public std::enable_shared_from_this<Session> {

public:
    static constexpr uint64_t CHALLENGE_EXPIRE_MS = 30'000;
    Session(tcp::socket socket, ServerWorld& server_world,
            asio::io_context& io);
    ~Session();
    void start();
    void send(Packet packet, int priority = 10);

    void close();
    const std::string& uuid() const;

    crypto::Ed25519PublicKey& public_key();
    std::optional<std::pair<uint64_t, crypto::Ed25519::Challenge>>& challenge();
    void set_player_uuid(std::optional<Uuid> uuid);

private:
    struct Task {
        int priority = 10;
        std::uint64_t sequence = 0;
        Packet packet;
        Task(int p, std::uint64_t seq, Packet pac)
            : priority(p), sequence(seq), packet(std::move(pac)) {}
    };

    struct TaskCompare {
        bool operator()(const Task& a, const Task& b) const {

            if (a.priority != b.priority) {
                return a.priority > b.priority;
            }

            return a.sequence > b.sequence;
        }
    };

    static constexpr uint32_t MAX_PACKET_SIZE = 4 * 1024 * 1024;
    tcp::socket m_socket;
    std::vector<char> m_read_buffer;
    std::priority_queue<Task, std::vector<Task>, TaskCompare> m_write_queue;
    asio::strand<asio::io_context::executor_type> m_strand;
    std::string m_uuid;

    crypto::Ed25519PublicKey m_public_key;
    std::optional<std::pair<uint64_t, crypto::Ed25519::Challenge>> m_challenge;
    std::optional<Uuid> m_player_uuid;
    ServerWorld& m_server_world;
    std::atomic<bool> m_closed{false};

    std::atomic_uint64_t m_sequence{0};

    asio::awaitable<void> read_loop();

    void do_write();
};
} // namespace cubed
