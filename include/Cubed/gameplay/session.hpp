#pragma once

#include "Cubed/gameplay/packet.hpp"

#include <asio.hpp>
#include <memory>
#include <queue>
#include <string>
namespace Cubed {

using asio::ip::tcp;
class ServerWorld;
class Session : public std::enable_shared_from_this<Session> {

public:
    Session(tcp::socket socket, ServerWorld& server_world,
            asio::io_context& io);
    ~Session();
    void start();
    void send(Packet packet, int priority = 10);

    void close();
    const std::string& uuid() const;

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
    ServerWorld& m_server_world;
    std::atomic<bool> m_closed{false};

    std::atomic_uint64_t m_sequence{0};

    asio::awaitable<void> read_loop();

    void do_write();
};
} // namespace Cubed
