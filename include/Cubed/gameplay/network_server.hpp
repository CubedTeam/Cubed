#pragma once
#include <asio.hpp>
#include <thread>
namespace Cubed {

class NetworkServer {
public:
    NetworkServer(int port = 25530);
    ~NetworkServer();
    void stop();
    void run();
    int port() const;

private:
    asio::io_context m_io;
    std::thread m_server;
    int m_port = 25530;
    asio::awaitable<void> listen();
};
} // namespace Cubed