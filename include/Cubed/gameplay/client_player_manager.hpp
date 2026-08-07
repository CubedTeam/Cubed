#pragma once
#include "Cubed/gameplay/client_player.hpp"
#include "Cubed/gameplay/local_player.hpp"
#include "Cubed/gameplay/network_client.hpp"
#include "Cubed/tools/sparse_vector.hpp"
namespace Cubed {
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

    void receive_remote_player(const PlayerInfoRsp& rsp);
    void receive_player_logout(const LogoutRsp& rsp);

    void reload_config();

    void report_player_info(NetworkClient* client);

private:
    ClientWorld& m_world;
    mutable std::shared_mutex m_players_mutex;
    using PlayerHandle = SparseVector<ClientPlayer>::Handle;
    SparseVector<ClientPlayer> m_players;
    std::vector<PlayerRenderData> m_render_data;
    std::unordered_map<std::string, PlayerHandle> m_players_handle;
    LocalPlayer m_local;

    void update_players_data(float dt);
};
} // namespace Cubed