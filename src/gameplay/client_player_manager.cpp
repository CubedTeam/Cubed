#include "Cubed/gameplay/client_player_manager.hpp"

#include "Cubed/gameplay/block_manager.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/tools/math_tools.hpp"
#include "Cubed/tools/time_tools.hpp"

#include <filesystem>
namespace fs = std::filesystem;

using namespace google::protobuf;

namespace Cubed {

namespace {
constexpr double ENTITY_RENDER_DELAY_MS = 100.0; // two tick time
constexpr size_t ENTITY_SNAPSHOT_MAX = 16;

// AI-generated: shortest-path angle lerp (degrees)
float lerp_angle(float from, float to, float t) {
    float delta = std::fmod(to - from, 360.0f);
    if (delta > 180.0f) {
        delta -= 360.0f;
    } else if (delta < -180.0f) {
        delta += 360.0f;
    }
    return from + delta * t;
}

ClientPlayerSnapshot
interpolate_snapshot(const std::deque<ClientPlayerSnapshot>& history,
                     double render_time) {

    if (history.empty()) {
        return {};
    }

    if (history.size() == 1) {
        return history.front();
    }

    auto upper = std::lower_bound(
        history.begin(), history.end(), render_time,
        [](const ClientPlayerSnapshot& s, double t) { return s.time_ms < t; });

    if (upper == history.end()) {
        return history.back();
    }

    if (upper == history.begin()) {
        return *upper;
    }

    auto lower = upper - 1;

    double span = upper->time_ms - lower->time_ms;

    float t = span > 0.0 ? float((render_time - lower->time_ms) / span) : 1.0f;

    t = std::clamp(t, 0.0f, 1.0f);

    ClientPlayerSnapshot out;

    out.pos = glm::mix(lower->pos, upper->pos, t);

    out.pitch = lerp_angle(lower->pitch, upper->pitch, t);

    out.yaw = lerp_angle(lower->yaw, upper->yaw, t);

    return out;
}
} // namespace

ClientPlayerManager::ClientPlayerManager(ClientWorld& world)
    : m_world(world), m_local(world) {}
ClientPlayerManager::~ClientPlayerManager() {

};

void ClientPlayerManager::init(std::string_view local_name) {
    m_local.init(local_name);
}

void ClientPlayerManager::update(float dt) {
    m_local.update(dt);

    update_players_data(dt);
}

bool ClientPlayerManager::has_player(const Hitbox& hitbox) const {
    std::shared_lock lock(m_players_mutex);

    for (const auto& player : m_players) {

        auto box = HitboxManager::hitbox("cubed:player");
        box.box.center += player.pos.value;
        if (box.box.intersects(hitbox)) {
            return true;
        }
    }
    return false;
}

void ClientPlayerManager::receive_remote_player(const PlayerInfoRsp& rsp) {
    auto pitch = rsp.pitch();
    auto yaw = rsp.yaw();
    {
        std::lock_guard lock(m_players_mutex);
        glm::vec3 pos{rsp.pos().x(), rsp.pos().y(), rsp.pos().z()};
        auto it = m_players_handle.find(rsp.uuid());
        if (it == m_players_handle.end()) {
            ClientPlayer data{};
            data.entity.name = rsp.name();
            data.entity.uuid = rsp.uuid();
            data.angle.pitch = pitch;
            data.angle.yaw = yaw;
            data.render_angle.pitch = pitch;
            data.render_angle.yaw = yaw;
            data.pos.value = pos;
            data.render_pos.value = pos;
            data.walk.gait = get_gait_from_id(rsp.gait());
            data.history.value.emplace_back(
                static_cast<double>(Tools::get_time_ticks()), pos, yaw, pitch);
            auto handle = m_players.emplace(std::move(data));
            m_players_handle.try_emplace(rsp.uuid(), handle);
        } else {
            auto it = m_players_handle.find(rsp.uuid());
            if (it != m_players_handle.end()) {
                auto& data = m_players[it->second];
                data.pos.value = pos;
                data.walk.gait = get_gait_from_id(rsp.gait());
                data.angle.yaw = yaw;
                data.angle.pitch = pitch;
                data.history.value.push_back(
                    {static_cast<double>(Tools::get_time_ticks()), pos, yaw,
                     pitch});
                while (data.history.value.size() > ENTITY_SNAPSHOT_MAX) {
                    data.history.value.pop_front();
                }
            }
        }
        // Logger::info("Player {} pos Update", rsp.name());
    }
}

void ClientPlayerManager::receive_player_logout(const LogoutRsp& rsp) {
    {
        std::lock_guard lock(m_players_mutex);
        auto it = m_players_handle.find(rsp.uuid());
        if (it == m_players_handle.end()) {
            Logger::warn("Player {} not find", rsp.uuid());
        } else {
            m_players.erase(it->second);
            m_players_handle.erase(it);
            Logger::info("Player {} erase", rsp.uuid());
        }
    }
}

void ClientPlayerManager::reload_config() { m_local.reload_config(); }

void ClientPlayerManager::report_player_info(NetworkClient* client) {
    if (!client) {
        return;
    }
    Arena arena;
    auto* info = Arena::Create<C2S_PlayerInfo>(&arena);
    info->set_uuid(m_local.get_uuid());
    glm::vec3 player_pos = m_local.get_player_pos();
    auto* v3 = info->mutable_pos();
    v3->set_x(player_pos.x);
    v3->set_y(player_pos.y);
    v3->set_z(player_pos.z);
    info->set_yaw(m_local.yaw());
    info->set_pitch(m_local.pitch());
    info->set_gait(get_gait_id(m_local.get_gait()));

    client->send(make_packet(*info), 0);
}

void ClientPlayerManager::update_players_data(float dt) {
    m_render_data.clear();
    auto m_rendering_distance = m_world.rendering_distance();
    auto update_renderinfo = [this, dt,
                              m_rendering_distance](ClientPlayer& player) {
        double render_time = static_cast<double>(Tools::get_time_ticks()) -
                             ENTITY_RENDER_DELAY_MS;
        auto snapshot = interpolate_snapshot(player.history.value, render_time);

        player.render_pos.value = snapshot.pos;

        player.render_angle.yaw = snapshot.yaw;

        player.render_angle.pitch = snapshot.pitch;

        if (player.walk.gait == Gait::WALK || player.walk.gait == Gait::RUN) {

            player.walk.walk_time += dt;

            float speed = player.walk.gait == Gait::RUN ? 14.0f : 8.0f;
            float amp = player.walk.gait == Gait::RUN ? 50.0f : 35.0f;
            // float amp = 90.0f;
            player.render_angle.roll =
                glm::sin(player.walk.walk_time * speed) * glm::radians(amp);
        } else if (player.walk.gait == Gait::STOP) {
            float t = glm::clamp(dt * 10.0f, 0.0f, 1.0f);
            player.render_angle.roll =
                glm::mix(player.render_angle.roll, 0.0f, t);
        }

        if (Math::distance2(player.render_pos.value, m_local.get_player_pos()) >
            m_rendering_distance * CHUNK_SIZE * m_rendering_distance *
                CHUNK_SIZE) {
            return;
        }

        // walking sound
        if (player.walk.gait == Gait::STOP) {
            player.walk.moving_time = 0.0f;
        } else {
            player.walk.moving_time += dt;
        }

        auto play_walk_sound = [&]() {
            glm::ivec3 block = glm::floor(player.render_pos.value);
            block.y -= 1;
            BlockType id = m_world.get_block_tpye(block);
            if (id == 0) {
                return;
            }
            auto data = BlockManager::data(id);
            if (data.sound.walk) {
                fs::path path = data.sound.walk->full_path();
                m_world.get_audio().play_3d(path, player.render_pos.value,
                                            true);
            }
        };

        if (player.walk.gait == Gait::WALK) {
            if (player.walk.moving_time >= LocalPlayer::WALK_SOUND_INTERVAL) {
                player.walk.moving_time = 0.0f;
                play_walk_sound();
            }
        }

        if (player.walk.gait == Gait::RUN) {
            if (player.walk.moving_time >= LocalPlayer::RUN_SOUND_INTERVAL) {
                player.walk.moving_time = 0.0f;
                play_walk_sound();
            }
        }

        PlayerRenderData render_data;
        render_data.render_pos = player.render_pos;
        render_data.angle = player.render_angle;
        render_data.gait = player.walk.gait;
        render_data.info = player.entity;
        m_render_data.emplace_back(std::move(render_data));
    };

    {
        std::lock_guard lock(m_players_mutex);

        for (auto& player : m_players) {
            update_renderinfo(player);
        }
        {
            auto gait = m_local.get_gait();
            auto& walk_time = m_local.walk_time();
            auto& angle = m_local.roll();
            if (gait == Gait::WALK || gait == Gait::RUN) {

                walk_time += dt;

                float speed = gait == Gait::RUN ? 14.0f : 8.0f;
                float amp = gait == Gait::RUN ? 50.0f : 35.0f;
                // float amp = 90.0f;
                angle = glm::sin(walk_time * speed) * glm::radians(amp);
            } else if (gait == Gait::STOP) {
                float t = glm::clamp(dt * 10.0f, 0.0f, 1.0f);
                angle = glm::mix(angle, 0.0f, t);
            }
            PlayerRenderData render_data{};
            render_data.render_pos.value = m_local.get_player_pos();
            render_data.angle.yaw = m_local.yaw();
            render_data.angle.pitch = m_local.pitch();
            render_data.angle.roll = angle;
            render_data.gait = m_local.get_gait();
            render_data.info.name = m_local.get_name();
            render_data.info.uuid = m_local.get_uuid();
            m_render_data.emplace_back(std::move(render_data));
        }
    }
}

LocalPlayer& ClientPlayerManager::get_local() { return m_local; }
const LocalPlayer& ClientPlayerManager::get_local() const { return m_local; }
std::span<PlayerRenderData> ClientPlayerManager::render_player_data() {
    return m_render_data;
}
} // namespace Cubed