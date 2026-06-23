#pragma once

#include <asio.hpp>
#include <deque>
#include <memory>
#include <string>
namespace Cubed {
using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {

public:
    Session(tcp::socket socket);
    ~Session();
    void start();
    void send();
    void close();
    const std::string& uuid() const;

private:
    static constexpr int HEADER_LEN = 8;
    static constexpr uint32_t MAX_PACKET_SIZE = 4 * 1024 * 1024;
    tcp::socket m_socket;
    std::vector<char> m_read_buffer;
    std::deque<std::vector<char>> m_write_queue;
    std::mutex m_write_mutex;

    std::string m_uuid;

    asio::awaitable<void> read();
    asio::awaitable<void> write();
};
} // namespace Cubed
