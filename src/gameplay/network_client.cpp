#include "Cubed/gameplay/network_client.hpp"

#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/net_error.hpp"

#include <utility>

using namespace google::protobuf;
namespace Cubed {
NetworkClient::NetworkClient(ClientWorld& world)
    : m_socket(m_io), m_strand(asio::make_strand(m_io)), m_world(world) {}

NetworkClient::~NetworkClient() { close(); }

void NetworkClient::start(std::string ip, int port) {
    if (m_net_thread.joinable()) {
        return;
    }
    m_net_thread = std::thread([self = shared_from_this(), ip, port]() {
        asio::co_spawn(self->m_strand, self->connect(ip, port), asio::detached);
        self->m_io.run();
    });
    Logger::info("NetworkClient Started");
}

bool NetworkClient::is_connected() const { return m_connected.load(); }
bool NetworkClient::is_connect_error() const { return m_connect_error.load(); }
std::string NetworkClient::get_error_string() const {
    std::lock_guard lock(m_error_string_mutex);
    return m_error_string;
}
void NetworkClient::clear_error() { m_connect_error = false; }
asio::awaitable<void> NetworkClient::connect(std::string ip, int port) {
    Logger::info("Connect Begin");
    try {
        auto ex = co_await asio::this_coro::executor;
        tcp::resolver resolver(ex);
        auto eps = co_await resolver.async_resolve(ip, std::to_string(port),
                                                   asio::use_awaitable);
        Logger::info("Resolve Success");
        co_await async_connect(m_socket, eps, asio::use_awaitable);
        Logger::info("Connect Success, Server ip {} port {}", ip, port);
        asio::co_spawn(m_strand, read_loop(), asio::detached);
        Logger::info("NetworkClient Read Loop Started");
        m_connected = true;
        co_return;
    } catch (const asio::system_error& e) {
        std::string_view error = net_error_message(e.code());
        Logger::error("Client Error: {}, code {}", error, e.code().value());
        set_error(error);
    } catch (const std::exception& e) {

        Logger::error("Client Error {}", e.what());
        set_error(e.what());
    }
}

asio::awaitable<void> NetworkClient::read_loop() {
    try {
        while (true) {
            std::array<uint8_t, HEADER_LEN> header_buffer;
            co_await asio::async_read(m_socket, asio::buffer(header_buffer),
                                      asio::use_awaitable);
            auto header = decode_packet_header(header_buffer);
            uint32_t total_len = HEADER_LEN + header.compressed_size;

            if (total_len < HEADER_LEN || total_len > MAX_PACKET_SIZE) {

                throw std::runtime_error("invalid packet length");
            }
            // maybe move, don't use it after switch!
            std::vector<uint8_t> body_data(header.compressed_size);
            if (header.compressed_size > 0) {
                co_await asio::async_read(m_socket, asio::buffer(body_data),
                                          asio::use_awaitable);
            }

            using std::to_underlying;
            Arena arena;
            switch (header.cmd) {
            case std::to_underlying(PacketEnum::LOGIN_RSP): {
                auto* rsp = Arena::Create<LoginRsp>(&arena);
                Logger::info("Client: Receive Login rsp");
                if (decode_packet(*rsp, body_data, header)) {
                    m_world.receive_login_rsp(*rsp);
                }
            } break;
            case std::to_underlying(PacketEnum::CHUNK_DATA_RSP): {
                // Logger::info("Client: Receive Chunk Data rsp, size {}mb",
                //              body_data.size() / 1024.0f / 1024);
                m_world.receive_chunk(std::move(body_data), header);
            } break;
            case std::to_underlying(PacketEnum::BLOCK_CHANGE_RSP): {
                auto* rsp = Arena::Create<BlockChangeRsp>(&arena);
                Logger::info("Client: Receive Block Change rsp");
                if (decode_packet(*rsp, body_data, header)) {
                    m_world.receive_block_change(*rsp);
                }
            } break;
            case std::to_underlying(PacketEnum::UPDATE_TIME): {
                auto* rsp = Arena::Create<UpdateTime>(&arena);
                if (decode_packet(*rsp, body_data, header)) {
                    m_world.receive_time(*rsp);
                }
            } break;
            case std::to_underlying(PacketEnum::PLAYER_INFO_RSP): {
                auto* rsp = Arena::Create<PlayerInfoRsp>(&arena);
                if (decode_packet(*rsp, body_data, header)) {
                    m_world.receive_remote_player(*rsp);
                }
            } break;
            case std::to_underlying(PacketEnum::LOGOUT_RSP): {
                auto* rsp = Arena::Create<LogoutRsp>(&arena);
                if (decode_packet(*rsp, body_data, header)) {
                    m_world.receive_player_logout(*rsp);
                }
            } break;
            case std::to_underlying(PacketEnum::S2C_CLEAR_ALL_CHUNKS): {
                auto* rsp = Arena::Create<S2C_ClearAllChunks>(&arena);
                if (decode_packet(*rsp, body_data, header)) {
                    if (rsp->clear()) {
                        Logger::info("Client Clear All Chunk");
                        m_world.rebuild_world();
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::PLAYER_WATER_SOUND): {
                auto* rsp = Arena::Create<PlayerWaterSound>(&arena);
                if (decode_packet(*rsp, body_data, header)) {
                    m_world.receive_player_water_sound(*rsp);
                }
            } break;
            case std::to_underlying(PacketEnum::CHAT_MSG): {
                auto* msg = Arena::Create<ChatMsg>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    m_world.receive_chat_message(*msg);
                }
            } break;
            case std::to_underlying(PacketEnum::VOICE_MSG): {
                auto* msg = Arena::Create<VoiceMsg>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    m_world.receive_voice_message(*msg);
                }
            }
            }
        }
    } catch (const asio::system_error& e) {
        std::string_view error = net_error_message(e.code());
        Logger::error("Client Error: {}, code {}", error, e.code().value());
        set_error(error);
        close();
    } catch (const std::exception& e) {
        Logger::error("Client Error {}", e.what());
        set_error(e.what());
        close();
    } catch (...) {
        Logger::error("Unknown Error");
        set_error("Unknown Error");
        close();
    }
    co_return;
}

void NetworkClient::send(Packet packet, int priority) {
    if (m_closed.load()) {
        return;
    }
    asio::post(m_strand, [self = shared_from_this(), packet = std::move(packet),
                          priority]() mutable {
        bool idle = self->m_write_queue.empty();
        self->m_write_queue.emplace(priority, self->m_sequence++,
                                    std::move(packet));
        if (idle) {
            self->do_write();
        }
    });
}

void NetworkClient::do_write() {
    if (m_closed.load()) {
        return;
    }

    auto self = shared_from_this();
    auto packet = std::move(m_write_queue.top().packet);
    asio::async_write(
        m_socket, asio::buffer(*packet),
        asio::bind_executor(m_strand, [self](std::error_code ec, size_t) {
            if (ec) {
                std::string_view error = net_error_message(ec);
                Logger::error("Cleint Write Error: {}, code {}", error,
                              ec.value());
                self->set_error(error);
                self->close();
                return;
            }
            self->m_write_queue.pop();
            if (!self->m_write_queue.empty()) {
                self->do_write();
            }
        }));
}

void NetworkClient::close() {
    if (m_closed.exchange(true)) {
        return;
    }

    std::error_code ec;

    m_socket.shutdown(tcp::socket::shutdown_both, ec);

    m_socket.close(ec);
    Logger::info("NetworkClient Closed");
    m_connected = false;
    m_io.stop();
}

void NetworkClient::stop() {
    close();

    if (m_net_thread.joinable()) {
        m_net_thread.join();
    }
}

void NetworkClient::set_error(std::string_view error) {
    std::lock_guard lock(m_error_string_mutex);
    m_error_string = error;
    m_connect_error = true;
}
ClientWorld& NetworkClient::world() { return m_world; }
} // namespace Cubed