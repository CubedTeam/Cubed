#include "Cubed/gameplay/session.hpp"

#include "Cubed/tools/log.hpp"
#include "Cubed/tools/uuid.hpp"
using asio::ip::tcp;

namespace Cubed {
Session::Session(tcp::socket socket)
    : m_socket(std::move(socket)), m_uuid(generate_uuid()) {}

Session::~Session() {}

void Session::start() {
    auto self = shared_from_this();
    asio::co_spawn(
        m_socket.get_executor(),
        [self]() -> asio::awaitable<void> { co_await self->read(); },
        asio::detached);
}

const std::string& Session::uuid() const { return m_uuid; }

asio::awaitable<void> Session::read() {
    try {
        while (true) {
            std::array<char, HEADER_LEN> header;
            co_await asio::async_read(m_socket, asio::buffer(header),
                                      asio::use_awaitable);

            uint32_t total_len_net;
            std::memcpy(&total_len_net, header.data(), sizeof(total_len_net));
            uint32_t total_len = ntohl(total_len_net);

            uint16_t cmd_id_net;
            std::memcpy(&cmd_id_net, header.data() + 4, sizeof(cmd_id_net));
            uint16_t cmd_id = ntohs(cmd_id_net);
            if (total_len < HEADER_LEN || total_len > MAX_PACKET_SIZE) {

                throw std::runtime_error("invalid packet length");
            }
            uint32_t body_len = total_len - HEADER_LEN;
            std::vector<char> body_data(body_len);
            if (body_len > 0) {
                co_await asio::async_read(m_socket, asio::buffer(body_data),
                                          asio::use_awaitable);
            }

            if (cmd_id == 1001) {
            }
        }
    } catch (const asio::system_error& e) {
        Logger::warn("Catch Asio Error {}", e.what());
        close();
    } catch (...) {
        Logger::error("Unknow Error");
    }
}

} // namespace Cubed