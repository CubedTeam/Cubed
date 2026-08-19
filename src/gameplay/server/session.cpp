#include "Cubed/gameplay/server/session.hpp"

#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/net_error.hpp"
#include "Cubed/tools/uuid.hpp"

#include <tracy/Tracy.hpp>
using asio::ip::tcp;
using namespace google::protobuf;
namespace cubed {
Session::Session(tcp::socket socket, ServerWorld& server_world,
                 asio::io_context& io)
    : m_socket(std::move(socket)), m_strand(asio::make_strand(io)),
      m_uuid(generate_uuid()), m_server_world(server_world) {}

Session::~Session() {}

void Session::start() {
    auto self = shared_from_this();
    asio::co_spawn(
        m_strand,
        [self]() -> asio::awaitable<void> { co_await self->read_loop(); },
        asio::detached);
}

void Session::send(std::shared_ptr<std::vector<uint8_t>> packet, int priority) {
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

const std::string& Session::uuid() const { return m_uuid; }

crypto::Ed25519PublicKey& Session::public_key() { return m_public_key; }
std::optional<std::pair<uint64_t, crypto::Ed25519::Challenge>>&
Session::challenge() {
    return m_challenge;
}
void Session::set_player_uuid(std::optional<Uuid> uuid) {
    auto self = shared_from_this();
    asio::dispatch(m_strand, [self, uuid = std::move(uuid)]() mutable {
        self->m_player_uuid = std::move(uuid);
    });
}
asio::awaitable<void> Session::read_loop() {
    ZoneScopedN("Session::read_loop");
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
            std::vector<uint8_t> body_data(header.compressed_size);
            if (header.compressed_size > 0) {
                co_await asio::async_read(m_socket, asio::buffer(body_data),
                                          asio::use_awaitable);
            }
            Arena arena;
            switch (header.cmd) {
            case std::to_underlying(PacketEnum::C2S_LOGIN_REQ): {
                if (!m_player_uuid) {
                    auto* req = Arena::Create<protocol::C2SLoginReq>(&arena);
                    Logger::info("Session: Receive Login req");
                    if (decode_packet(*req, body_data, header)) {
                        m_server_world.handle_player_login(*req,
                                                           shared_from_this());
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::C2S_PLAYER_INFO): {
                auto* pos = Arena::Create<protocol::C2SPlayerInfo>(&arena);
                if (decode_packet(*pos, body_data, header)) {
                    if (m_player_uuid &&
                        m_player_uuid == Uuid::from_proto_bytes(pos->uuid())) {
                        m_server_world.sync_player_pos(*pos);
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::C2S_CHUNK_DATA_REQ): {
                auto* req = Arena::Create<protocol::C2SChunkDataReq>(&arena);
                // Logger::info("Session: Receive Chunk Data req");
                if (decode_packet(*req, body_data, header)) {
                    auto uuid = Uuid::from_proto_bytes(req->uuid());

                    if (!uuid) {
                        Logger::error("Can't parse uuid from proto");
                    } else {
                        if (m_player_uuid && m_player_uuid == uuid) {
                            m_server_world.handle_chunk_req(
                                req->task_id(), *uuid,
                                ChunkPos(req->pos().x(), req->pos().z()));
                        }
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::C2S_BLOCK_CHANGE_REQ): {
                auto* req = Arena::Create<protocol::C2SBlockChangeReq>(&arena);

                if (decode_packet(*req, body_data, header)) {
                    if (m_player_uuid &&
                        m_player_uuid == Uuid::from_proto_bytes(req->uuid())) {
                        m_server_world.handle_block_change(*req);
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::C2S_LOGOUT_REQ): {
                auto* req = Arena::Create<protocol::C2SLogoutReq>(&arena);
                if (decode_packet(*req, body_data, header)) {
                    auto uuid = Uuid::from_proto_bytes(req->uuid());
                    if (!uuid) {
                        Logger::error("Can't parse uuid from proto");
                    } else {
                        if (m_player_uuid && m_player_uuid == uuid) {
                            auto player =
                                m_server_world.player_manager().find(*uuid);
                            if (player &&
                                player->get_session() == shared_from_this()) {
                                m_server_world.handle_player_exit(
                                    std::move(player));
                            }
                        }
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::S2C_PLAYER_WATER_SOUND): {
                auto* req =
                    Arena::Create<protocol::S2CPlayerWaterSound>(&arena);
                if (decode_packet(*req, body_data, header)) {
                    if (m_player_uuid &&
                        m_player_uuid == Uuid::from_proto_bytes(req->uuid())) {
                        m_server_world.sync_player_water_sound(*req);
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::CHAT_MSG): {
                auto* msg = Arena::Create<protocol::ChatMsg>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    if (m_player_uuid &&
                        m_player_uuid == Uuid::from_proto_bytes(msg->uuid())) {
                        m_server_world.handle_chat_message(*msg);
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::VOICE_MSG): {
                auto* msg = Arena::Create<protocol::VoiceMsg>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    if (m_player_uuid &&
                        m_player_uuid == Uuid::from_proto_bytes(msg->uuid())) {
                        m_server_world.handle_voice_message(*msg);
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::C2S_ENTITY_CREATE_REQ): {
                auto* msg = Arena::Create<protocol::C2SEntityCreateReq>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    if (m_player_uuid &&
                        m_player_uuid == Uuid::from_proto_bytes(msg->uuid())) {
                        m_server_world.handle_entity_create(*msg);
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::C2S_ENTITY_DESTROY_REQ): {
                auto* msg =
                    Arena::Create<protocol::C2SEntityDestroyReq>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    if (m_player_uuid &&
                        m_player_uuid == Uuid::from_proto_bytes(msg->uuid())) {
                        m_server_world.handle_entity_destroy(*msg);
                    }
                }
            } break;
            case std::to_underlying(PacketEnum::C2S_LOGIN_PROOF): {
                auto* msg = Arena::Create<protocol::C2SLoginProof>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    m_server_world.handle_login_proof(*msg, shared_from_this());
                }
            } break;
            case std::to_underlying(PacketEnum::PING): {
                auto* msg = Arena::Create<protocol::Ping>(&arena);
                if (decode_packet(*msg, body_data, header)) {
                    m_server_world.handle_ping(*msg, shared_from_this());
                }
            } break;
            }
        }
    } catch (const asio::system_error& e) {
        auto ec = e.code();

        if (ec == asio::error::eof || ec == asio::error::operation_aborted) {
            Logger::info("Client disconnected");
        } else {
            std::string_view error = net_error_message(e.code());
            Logger::error("Server Error: {}, code {}", error, e.code().value());
        }

        close();
    } catch (const std::exception& e) {
        Logger::error("Session Error {}", e.what());
        close();
    } catch (...) {
        Logger::error("Unknow Error");
        close();
    }
    co_return;
}

void Session::do_write() {

    auto self = shared_from_this();
    auto packet = std::move(m_write_queue.top().packet);
    asio::async_write(
        m_socket, asio::buffer(*packet),
        asio::bind_executor(m_strand, [self](std::error_code ec, size_t) {
            if (ec) {
                std::string_view error = net_error_message(ec);
                Logger::error("Server Error: {}, code {}", error, ec.value());
                self->close();
                return;
            }
            self->m_write_queue.pop();
            if (!self->m_write_queue.empty()) {
                self->do_write();
            }
        }));
}

void Session::close() {
    if (m_closed.exchange(true)) {
        return;
    }

    std::error_code ec;

    m_socket.shutdown(tcp::socket::shutdown_both, ec);

    m_socket.close(ec);
}

} // namespace cubed
