#pragma once
#include "Cubed/gameplay/server_world.hpp"
#include "Cubed/gameplay/session.hpp"

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
    std::unordered_map<std::string, std::shared_ptr<Session>> m_session;

private:
    asio::io_context m_io;
    std::thread m_server;
    int m_port = 25530;
    std::atomic<bool> m_stopped{false};
    ServerWorld m_world;
    asio::awaitable<void> listen();
};
} // namespace Cubed