#pragma once

#include "Cubed/gameplay/packet.hpp"

#include <asio.hpp>
#include <queue>
#include <string>
#include <thread>
namespace Cubed {
using asio::ip::tcp;
class ClientWorld;
class NetworkClient : public std::enable_shared_from_this<NetworkClient> {
public:
    NetworkClient(ClientWorld& world);
    ~NetworkClient();
    void close();
    void stop();
    void send(Packet packet, int priority = 10);
    void start(std::string ip, int port = 25530);
    bool is_connected() const;
    bool is_connect_error() const;

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

    asio::io_context m_io;

    std::thread m_net_thread;
    static constexpr uint32_t MAX_PACKET_SIZE = 4 * 1024 * 1024;
    tcp::socket m_socket;
    std::vector<char> m_read_buffer;

    std::priority_queue<Task, std::vector<Task>, TaskCompare> m_write_queue;

    asio::strand<asio::io_context::executor_type> m_strand;
    std::atomic<bool> m_closed{false};
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_connect_error{false};
    // ClientWorld is managed by App
    ClientWorld& m_world;
    std::atomic_uint64_t m_sequence{0};

    asio::awaitable<void> connect(std::string ip, int port);
    asio::awaitable<void> read_loop();

    void do_write();
};
} // namespace Cubed