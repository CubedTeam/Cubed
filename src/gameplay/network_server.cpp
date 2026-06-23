#include "Cubed/gameplay/network_server.hpp"

#include "Cubed/tools/log.hpp"
using asio::ip::tcp;
namespace Cubed {

NetworkServer::NetworkServer(int port) : m_port(port) {}

NetworkServer::~NetworkServer() { stop(); }

void NetworkServer::stop() {
    m_io.stop();
    if (m_server.joinable()) {
        m_server.join();
    }
    Logger::info("Server Stopped!");
}

asio::awaitable<void> NetworkServer::listen() {
    tcp::acceptor acceptor(m_io, tcp::endpoint(tcp::v4(), m_port));
    while (true) {

        tcp::socket socket =
            co_await acceptor.async_accept(asio::use_awaitable);
    }
}

void NetworkServer::run() {
    m_server = std::thread([this]() {
        asio::co_spawn(m_io, listen(), asio::detached);
        m_io.run();
    });
    Logger::info("Server Started!");
}

int NetworkServer::port() const { return m_port; }

} // namespace Cubed