#pragma once
#include "Cubed/gameplay/client/client_player.hpp"
#include "Cubed/gameplay/client/local_player.hpp"
#include "Cubed/gameplay/client/network_client.hpp"
#include "Cubed/tools/sparse_vector.hpp"
namespace cubed {
class ClientWorld;
class ClientPlayerManager {
public:
    ClientPlayerManager(const ClientPlayerManager&) = delete;
    ClientPlayerManager(ClientPlayerManager&&) = delete;
    ClientPlayerManager& operator=(const ClientPlayerManager&) = delete;
    ClientPlayerManager& operator=(ClientPlayerManager&&) = delete;
    ClientPlayerManager(ClientWorld& world);
    ~ClientPlayerManager();

    void init(std::string_view local_name);
    void update(float dt);

    std::span<PlayerRenderData> render_player_data();

    bool has_player(const Hitbox& hitbox) const;

    LocalPlayer& get_local();
    const LocalPlayer& get_local() const;

    void receive_remote_player(const protocol::S2CPlayerInfoRsp& rsp);
    void receive_player_logout(const protocol::S2CLogoutRsp& rsp);

    void reload_config();

    void report_player_info(NetworkClient* client);

private:
    ClientWorld& m_world;
    mutable std::shared_mutex m_players_mutex;
    using PlayerHandle = SparseVector<ClientPlayer>::Handle;
    SparseVector<ClientPlayer> m_players;
    std::vector<PlayerRenderData> m_render_data;
    std::unordered_map<Uuid, PlayerHandle> m_players_handle;
    LocalPlayer m_local;

    void update_players_data(float dt);
};
} // namespace cubed
