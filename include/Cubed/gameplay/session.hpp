#pragma once

#include <asio.hpp>
#include <deque>
#include <memory>
#include <string>
namespace Cubed {
using asio::ip::tcp;
class ServerWorld;
class Session : public std::enable_shared_from_this<Session> {

public:
    Session(tcp::socket socket, ServerWorld& server_world);
    ~Session();
    void start();
    void send(std::shared_ptr<std::vector<uint8_t>> packet);
    void close();
    const std::string& uuid() const;

private:
    static constexpr int HEADER_LEN = 8;
    static constexpr uint32_t MAX_PACKET_SIZE = 4 * 1024 * 1024;
    tcp::socket m_socket;
    std::vector<char> m_read_buffer;
    std::deque<std::shared_ptr<std::vector<uint8_t>>> m_write_queue;
    asio::strand<asio::io_context::executor_type> m_strand;
    std::string m_uuid;
    ServerWorld& m_server_world;
    asio::awaitable<void> read_loop();
    std::atomic<bool> m_closed{false};
    void do_write();
};
} // namespace Cubed
