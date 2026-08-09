#include "Cubed/gameplay/local_player.hpp"

#include "Cubed/audio/audio_engine.hpp"
#include "Cubed/config.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/gameplay/block_manager.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/gameplay/hitbox_manager.hpp"
#include "Cubed/gameplay/item_manager.hpp"

#include <filesystem>
namespace fs = std::filesystem;
namespace Cubed {

namespace {

bool update_x(const glm::vec3& pos, const glm::vec3& distance,
              ClientWorld& world, const Hitbox& box) {
    glm::vec3 p = pos;
    p.x += distance.x;
    Hitbox b = box;
    b.center += p;
    glm::vec3 min = b.min();
    glm::vec3 max = b.max();
    int minx = std::floor(min.x);
    int maxx = std::floor(max.x);
    int miny = std::floor(min.y);
    int maxy = std::floor(max.y);
    int minz = std::floor(min.z);
    int maxz = std::floor(max.z);

    for (int x = minx; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            for (int z = minz; z <= maxz; ++z) {
                glm::ivec3 block_pos{x, y, z};
                if (!world.can_pass_block(block_pos)) {
                    Hitbox block_box = World::get_block_aabb(block_pos);
                    if (b.intersects(block_box)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool update_y(const glm::vec3& pos, const glm::vec3& distance,
              ClientWorld& world, const Hitbox& box) {
    glm::vec3 p = pos;
    p.y += distance.y;
    Hitbox b = box;
    b.center += p;
    glm::vec3 min = b.min();
    glm::vec3 max = b.max();
    int minx = std::floor(min.x);
    int maxx = std::floor(max.x);
    int miny = std::floor(min.y);
    int maxy = std::floor(max.y);
    int minz = std::floor(min.z);
    int maxz = std::floor(max.z);

    for (int x = minx; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            for (int z = minz; z <= maxz; ++z) {
                glm::ivec3 block_pos{x, y, z};
                if (!world.can_pass_block(block_pos)) {
                    Hitbox block_box = World::get_block_aabb(block_pos);
                    if (b.intersects(block_box)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool update_z(const glm::vec3& pos, const glm::vec3& distance,
              ClientWorld& world, const Hitbox& box) {
    glm::vec3 p = pos;
    p.z += distance.z;
    Hitbox b = box;
    b.center += p;
    glm::vec3 min = b.min();
    glm::vec3 max = b.max();
    int minx = std::floor(min.x);
    int maxx = std::floor(max.x);
    int miny = std::floor(min.y);
    int maxy = std::floor(max.y);
    int minz = std::floor(min.z);
    int maxz = std::floor(max.z);

    for (int x = minx; x <= maxx; ++x) {
        for (int y = miny; y <= maxy; ++y) {
            for (int z = minz; z <= maxz; ++z) {
                glm::ivec3 block_pos{x, y, z};
                if (!world.can_pass_block(block_pos)) {
                    Hitbox block_box = World::get_block_aabb(block_pos);
                    if (b.intersects(block_box)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

} // namespace

LocalPlayer::LocalPlayer(ClientWorld& world) : m_world(world) {

    m_hitbox = HitboxManager::instance().get_hitbox_id("cubed:player");
}
LocalPlayer::~LocalPlayer() {}

const glm::vec3& LocalPlayer::get_front() const { return m_front; }

const std::optional<LookBlock>& LocalPlayer::get_look_block_pos() const {
    return m_look_block;
}
glm::vec3 LocalPlayer::get_player_pos() const {

    std::shared_lock lock(m_player_pos_mutex);
    return m_pos.value;
}

const MoveState& LocalPlayer::get_move_state() const { return m_move_state; }

bool LocalPlayer::ray_cast(const glm::vec3& start, const glm::vec3& front,
                           glm::ivec3& block_pos, glm::vec3& normal,
                           float distance) {
    glm::vec3 dir = glm::normalize(front);
    // float step = 0.1f;
    glm::ivec3 cur = glm::floor(start);
    int ix = cur.x;
    int iy = cur.y;
    int iz = cur.z;
    // step direction
    int step_x = (dir.x > 0) ? 1 : ((dir.x < 0) ? -1 : 0);
    int step_y = (dir.y > 0) ? 1 : ((dir.y < 0) ? -1 : 0);
    int step_z = (dir.z > 0) ? 1 : ((dir.z < 0) ? -1 : 0);

    static const float INF = std::numeric_limits<float>::infinity();

    float t_delta_x = (dir.x != 0) ? std::fabs(1.0f / dir.x) : INF;
    float t_delta_y = (dir.y != 0) ? std::fabs(1.0f / dir.y) : INF;
    float t_delta_z = (dir.z != 0) ? std::fabs(1.0f / dir.z) : INF;

    float t_max_x, t_max_y, t_max_z;

    if (dir.x > 0) {
        t_max_x = (static_cast<float>(ix) + 1.0f - start.x) / dir.x;
    } else if (dir.x < 0) {
        t_max_x = (start.x - static_cast<float>(ix)) / (-dir.x);
    } else {
        t_max_x = INF;
    }

    if (dir.y > 0) {
        t_max_y = (static_cast<float>(iy) + 1.0f - start.y) / dir.y;
    } else if (dir.y < 0) {
        t_max_y = (start.y - static_cast<float>(iy)) / (-dir.y);
    } else {
        t_max_y = INF;
    }

    if (dir.z > 0) {
        t_max_z = (static_cast<float>(iz) + 1.0f - start.z) / dir.z;
    } else if (dir.z < 0) {
        t_max_z = (start.z - static_cast<float>(iz)) / (-dir.z);
    } else {
        t_max_z = INF;
    }
    float t = 0.0f;
    normal = glm::vec3(0.0f, 0.0f, 0.0f);
    while (t <= distance) {
        if (m_world.is_solid(glm::ivec3(ix, iy, iz))) {
            block_pos = glm::ivec3(ix, iy, iz);
            return true;
        }

        if (t_max_x < t_max_y && t_max_x < t_max_z) {
            t = t_max_x;
            t_max_x += t_delta_x;
            normal = glm::vec3(-step_x, 0.0f, 0.0f);
            ix += step_x;
        } else if (t_max_y < t_max_z) {
            t = t_max_y;
            t_max_y += t_delta_y;
            normal = glm::vec3(0.0f, -step_y, 0.0f);
            iy += step_y;
        } else {
            t = t_max_z;
            t_max_z += t_delta_z;
            normal = glm::vec3(0.0f, 0.0f, -step_z);
            iz += step_z;
        }
    }
    return false;
}

void LocalPlayer::change_mode(GameMode mode) {
    m_game_mode = mode;
    Logger::info("Change GameMode to {}", to_str(mode));
    if (mode == CREATIVE) {
        m_move_state.is_fly = false;
        m_max_run_speed = DEFAULT_MAX_RUN_SPEED;
        m_velocity.max =
            glm::vec3{m_max_walk_speed, m_max_y_speed, m_max_walk_speed};
    } else if (mode == SPECTATOR) {
        m_move_state.is_fly = true;
        m_walk_pose.gait = Gait::RUN;
        m_velocity.max =
            glm::vec3{m_max_run_speed, m_max_y_speed, m_max_run_speed};
    }
}
void LocalPlayer::reload_config() {
    auto& config = m_world.get_config();
    m_sensitivity = config.get("player.mouse_sensitivity", 0.15f);
}
void LocalPlayer::set_player_pos(const glm::vec3& pos) {

    std::lock_guard lock(m_player_pos_mutex);
    m_pos.value = pos;
}

void LocalPlayer::update(float delta_time) {
    WalkPose pos = m_walk_pose;
    pos.gait = compute_gait();
    m_walk_pose = pos;
    update_move(delta_time);
    update_lookup_block();
    place_block(delta_time);
    std::shared_lock lock(m_player_pos_mutex);
    d_rep("player_pos", "x: {:.2f} y: {:.2f} z: {:.2f}", m_pos.value.x,
          m_pos.value.y, m_pos.value.z);
    d_rep("speed", "Speed(x, y, z): {:.2} m/s {:.2} m/s {:.2} m/s",
          m_velocity.value.x, m_velocity.value.y, m_velocity.value.z);
}
bool LocalPlayer::update_player_move_state(Key key, KeyAction action) {
    if (key == Key::W) {
        if (action == KeyAction::PRESS) {
            m_move_state.forward = true;
        }
        if (action == KeyAction::RELEASE) {
            m_move_state.forward = false;
            m_sprinting = false;
        }
    } else if (key == Key::S) {
        if (action == KeyAction::PRESS) {
            m_move_state.back = true;
        }
        if (action == KeyAction::RELEASE) {
            m_move_state.back = false;
        }
    } else if (key == Key::A) {
        if (action == KeyAction::PRESS) {
            m_move_state.left = true;
        }
        if (action == KeyAction::RELEASE) {
            m_move_state.left = false;
        }
    } else if (key == Key::D) {
        if (action == KeyAction::PRESS) {
            m_move_state.right = true;
        }
        if (action == KeyAction::RELEASE) {
            m_move_state.right = false;
        }
    } else if (key == Key::SPACE) {
        if (action == KeyAction::PRESS) {
            m_move_state.up = true;
            if (space_on) {
                if (m_game_mode == CREATIVE) {
                    m_move_state.is_fly = !m_move_state.is_fly;
                    m_velocity.value.y = 0.0f;
                }
                space_on = false;
                space_on_time = 0.0f;
            } else {
                space_on = true;
            }
        }
        if (action == KeyAction::RELEASE) {
            m_move_state.up = false;
        }
    } else if (key == Key::LEFT_SHIFT) {
        if (action == KeyAction::PRESS) {
            m_move_state.down = true;
        }
        if (action == KeyAction::RELEASE) {
            m_move_state.down = false;
        }
    } else if (key == Key::LEFT_CTRL) {
        if (action == KeyAction::PRESS) {

            m_sprinting = true;
        }
        /*
        if (action == KeyAction::RELEASE) {
            m_sprinting = false;
        }*/
    } else if (key == Key::F4) {
        if (action == KeyAction::PRESS) {
            if (m_game_mode == CREATIVE) {
                change_mode(SPECTATOR);
            } else {
                change_mode(CREATIVE);
            }
        }
    } else if (key == Key::NUMPAD_1 || key == Key::DIGIT_1) {
        m_selected_hotbar = 0;
    } else if (key == Key::NUMPAD_2 || key == Key::DIGIT_2) {
        m_selected_hotbar = 1;
    } else if (key == Key::NUMPAD_3 || key == Key::DIGIT_3) {
        m_selected_hotbar = 2;
    } else if (key == Key::NUMPAD_4 || key == Key::DIGIT_4) {
        m_selected_hotbar = 3;
    } else if (key == Key::NUMPAD_5 || key == Key::DIGIT_5) {
        m_selected_hotbar = 4;
    } else if (key == Key::NUMPAD_6 || key == Key::DIGIT_6) {
        m_selected_hotbar = 5;
    } else if (key == Key::NUMPAD_7 || key == Key::DIGIT_7) {
        m_selected_hotbar = 6;
    } else if (key == Key::NUMPAD_8 || key == Key::DIGIT_8) {
        m_selected_hotbar = 7;
    } else if (key == Key::NUMPAD_9 || key == Key::DIGIT_9) {
        m_selected_hotbar = 8;
    } else if (key == Key::NUMPAD_0 || key == Key::DIGIT_0) {
        m_selected_hotbar = 9;
    } else {
        return false;
    }

    m_moving = m_move_state.forward || m_move_state.back || m_move_state.left ||
               m_move_state.right;
    return true;
}

void LocalPlayer::update_front_vec(float offset_x, float offset_y) {
    Orientation& angle = m_angle;
    angle.yaw += offset_x * m_sensitivity;
    angle.pitch += offset_y * m_sensitivity;

    // m_yaw = std::fmod(m_yaw.load(), 360.0);

    angle.pitch = std::clamp(angle.pitch, -89.0f, 89.0f);

    m_front.x = sin(glm::radians(angle.yaw)) * cos(glm::radians(angle.pitch));
    m_front.y = sin(glm::radians(angle.pitch));
    m_front.z = -cos(glm::radians(angle.yaw)) * cos(glm::radians(angle.pitch));

    m_front = glm::normalize(m_front);
}

void LocalPlayer::update_direction() {
    m_right = glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f)));

    glm::vec3 move_dir_front = glm::vec3(0.0f);
    glm::vec3 move_dir_right = glm::vec3(0.0f);
    glm::vec3 move_dir = glm::vec3(0.0f);
    if (m_move_state.forward) {
        move_dir_front += glm::normalize(glm::vec3(m_front.x, 0.0f, m_front.z));
    }
    if (m_move_state.back) {
        move_dir_front -= glm::normalize(glm::vec3(m_front.x, 0.0f, m_front.z));
    }
    if (m_move_state.left) {
        move_dir_right -= glm::normalize(glm::vec3(m_right.x, 0.0f, m_right.z));
    }
    if (m_move_state.right) {
        move_dir_right += glm::normalize(glm::vec3(m_right.x, 0.0f, m_right.z));
    }
    move_dir = move_dir_front + move_dir_right;

    if (glm::length(move_dir) > 0.001f) {
        m_direction.value = glm::normalize(move_dir);
    }
}

void LocalPlayer::update_lookup_block() {
    // calculate the block that is looked
    glm::ivec3 block_pos;
    glm::vec3 block_normal;
    std::shared_lock lock(m_player_pos_mutex);
    if (ray_cast(
            glm::vec3(m_pos.value.x, (m_pos.value.y + 1.6f), m_pos.value.z),
            m_front, block_pos, block_normal)) {
        m_look_block = LookBlock{block_pos, glm::floor(block_normal)};
    } else {
        m_look_block = std::nullopt;
    }
}
void LocalPlayer::place_block(float dt) {

    if (m_look_block == std::nullopt) {
        return;
    }
    m_place_time += dt;
    if (m_place_time < PLACE_BLOCK_INTERVAL) {

        return;
    }
    m_place_time = 0.0f;
    if (m_mouse_state.left) {
        if (m_world.is_solid(m_look_block->pos)) {
            m_world.report_block_change(m_look_block->pos, 0);
        }
    }
    if (m_mouse_state.right) {
        auto data = ItemManager::get(m_hotbar[m_selected_hotbar].id);
        if (data.kind == ItemKind::BLOCK) {
            auto* t = std::get_if<BlockType>(&data.property);
            ASSERT(t);
            auto type = *t;
            if (type != 0) {
                glm::ivec3 near_pos = m_look_block->pos + m_look_block->normal;
                if (!m_world.is_solid(near_pos)) {
                    Hitbox block_box = ClientWorld::get_block_aabb(near_pos);
                    auto player_box = HitboxManager::hitbox("cubed:player");
                    player_box.box.center += get_player_pos();
                    if (!player_box.box.intersects(block_box)) {
                        m_world.report_block_change(near_pos, type);
                    }
                }
            }
        }
        if (data.kind == ItemKind::SPAWN_EGG) {
            auto* name = std::get_if<ResourceLocation>(&data.property);
            ASSERT(name);
            glm::ivec3 near_pos = m_look_block->pos + m_look_block->normal;
            if (!m_world.is_solid(near_pos)) {
                m_world.entity_manager().create(name->to_string(), near_pos);
            }
        }
    }
}

int LocalPlayer::selected_hotbar() const { return m_selected_hotbar; }
void LocalPlayer::set_hotbar(int pos, const ItemStack& item) {
    ASSERT(pos >= 0 && static_cast<size_t>(pos) < HOTBAR_SUM);
    m_hotbar[pos] = item;
}
std::span<const ItemStack, LocalPlayer::HOTBAR_SUM>
LocalPlayer::get_hotbar() const {
    return m_hotbar;
}

void LocalPlayer::update_move(float dt) {
    // if frame rate less than 1 frame per second, don't update
    if (dt > 1.0f) {
        return;
    }

    if (m_velocity.value.x < 0.01f || m_velocity.value.z < 0.01f) {
        m_sprinting = false;
    }
    if (space_on) {
        space_on_time += dt;
        if (space_on_time >= MAX_SPACE_ON_TIME) {
            space_on = false;
            space_on_time = 0.0f;
        }
    }

    if (m_game_mode != SPECTATOR) {
        m_velocity.max =
            (m_walk_pose.gait == Gait::RUN)
                ? glm::vec3{m_max_run_speed, m_max_y_speed, m_max_run_speed}
                : glm::vec3{m_max_walk_speed, m_max_y_speed, m_max_walk_speed};
    } else {
        m_velocity.max =
            glm::vec3{m_max_run_speed, m_max_y_speed, m_max_run_speed};
    }

    update_speed(dt);

    update_direction();

    // ensure the thread safe
    glm::vec3 player_pos;

    {
        std::shared_lock lock(m_player_pos_mutex);
        player_pos = m_pos.value;
    }

    if (m_game_mode == SPECTATOR) {
        player_pos += get_move_distance(dt);
    } else {
        auto [x, y, z] = update_physical(dt, player_pos);
        if (!x || !z) {
            m_sprinting = false;
        }
    }

    if (player_pos.y < -15.0f) {
        Logger::warn("y is tow low");
        player_pos += glm::vec3(1.0f, 100.0f, 1.0f);
    }

    {
        std::lock_guard lock(m_player_pos_mutex);
        m_pos.value = player_pos;
    }
    update_player_chunk();
    auto it = m_timers.find("Player Walk Sound");

    if (it != m_timers.end()) {
        if (m_sprinting) {
            it->second.set_threshold(RUN_SOUND_INTERVAL);
        } else {
            it->second.set_threshold(WALK_SOUND_INTERVAL);
        }
    }

    for (auto& [key, timer] : m_timers) {
        timer.update(dt);
    }
}

void LocalPlayer::update_player_chunk() {
    float x, z;
    {
        std::shared_lock lock(m_player_pos_mutex);
        x = m_pos.value.x;
        z = m_pos.value.z;
    }
    ChunkPos chunk_pos = get_chunk_pos(x, z);
    float dist = distance2(chunk_pos, m_last_chunk_pos);
    if (dist > 2) {
        Logger::info("Player request new chunk");
        m_world.request_chunk();
        m_last_chunk_pos = chunk_pos;
    }
}

Gait LocalPlayer::compute_gait() const {
    if (m_velocity.value.x < 0.01f && m_velocity.value.z < 0.01f)
        return Gait::STOP;

    if (m_sprinting)
        return Gait::RUN;

    return Gait::WALK;
}

bool LocalPlayer::update_scroll(float yoffset) {
    if (m_game_mode == SPECTATOR) {
        if (yoffset > 0) {
            if (m_max_run_speed < 500.0f) {
                m_max_run_speed += 1.0f;
            }
        } else {
            if (m_max_run_speed > 1.0f) {
                m_max_run_speed -= 1.0f;
            }
        }
    }
    if (m_game_mode == CREATIVE) {
        if (yoffset < 0) {
            m_selected_hotbar += 1;
            if (m_selected_hotbar >= 10) {
                m_selected_hotbar = 0;
            }
        } else {
            m_selected_hotbar -= 1;
            if (m_selected_hotbar < 0) {
                m_selected_hotbar = 0;
            }
        }
    }
    return true;
}

bool LocalPlayer::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS) {
        if (e.key == MouseKey::LEFT_BUTTON) {
            m_mouse_state.left = true;
            m_place_time = PLACE_BLOCK_INTERVAL;
            return true;
        }
        if (e.key == MouseKey::RIGHT_BUTTON) {
            m_mouse_state.right = true;
            m_place_time = PLACE_BLOCK_INTERVAL;
            return true;
        }
    }
    if (e.action == KeyAction::RELEASE) {
        if (e.key == MouseKey::LEFT_BUTTON) {
            m_mouse_state.left = false;
            return true;
        }
        if (e.key == MouseKey::RIGHT_BUTTON) {
            m_mouse_state.right = false;
            return true;
        }
    }
    return false;
}

bool LocalPlayer::handle_key_event(const KeyEvent& e) {

    if (update_player_move_state(e.key, e.action)) {
        return true;
    }

    return false;
}
bool LocalPlayer::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    if (update_scroll(e.offset)) {
        return true;
    }
    return false;
}

void LocalPlayer::update_chunk_set(const ChunkPosSet& set) {
    std::lock_guard lock(m_chunk_pos_mutex);
    m_player_chunk_pos_set.clear();
    m_player_chunk_pos_set.insert(set.begin(), set.end());
}

const LocalPlayer::ChunkPosSet& LocalPlayer::get_chunk_pos_set() const {
    std::shared_lock lock(m_chunk_pos_mutex);
    return m_player_chunk_pos_set;
}

LocalPlayer::ChunkPosSet LocalPlayer::get_chunk_pos_set() {
    std::lock_guard lock(m_chunk_pos_mutex);
    return m_player_chunk_pos_set;
}

float& LocalPlayer::max_walk_speed() { return m_max_walk_speed; }
float& LocalPlayer::max_run_speed() { return m_max_run_speed; }
float& LocalPlayer::fly_y_speed() { return m_max_y_speed; }
const ItemStack& LocalPlayer::get_current_itemstack() const {
    return m_hotbar[m_selected_hotbar];
};

GameMode& LocalPlayer::game_mode() { return m_game_mode; }
ClientWorld& LocalPlayer::get_world() { return m_world; }

void LocalPlayer::set_uuid(std::string_view uuid) {
    std::lock_guard lock(m_uuid_mutex);
    m_uuid = uuid;
}
std::string LocalPlayer::get_uuid() const {

    std::shared_lock lock(m_uuid_mutex);
    return m_uuid;
}
const std::string& LocalPlayer::get_name() const { return m_name; }

void LocalPlayer::reset_input_status() {
    m_mouse_state.left = false;
    m_mouse_state.right = false;
    m_move_state.left = false;
    m_move_state.right = false;
    m_move_state.back = false;
    m_move_state.forward = false;
    m_move_state.down = false;
    m_move_state.up = false;
}

void LocalPlayer::init(std::string_view name) {

    m_hitbox = HitboxManager::hitbox("cubed:player").id;

    {
        std::lock_guard lock(m_player_pos_mutex);
        m_pos.value = {0.0f, 255.0f, 0.0f};
    }

    m_name = name;

    m_timers.try_emplace("Player Walk Sound", WALK_SOUND_INTERVAL, [this]() {
        if (!m_moving || m_move_state.is_fly) {
            return;
        }
        glm::ivec3 block;
        glm::vec3 pos;
        {
            std::shared_lock lock(m_player_pos_mutex);
            block = glm::floor(m_pos.value);
            pos = m_pos.value;
        }

        block.y -= 1;
        BlockType id = m_world.get_block_tpye(block);
        if (id == 0) {
            return;
        }
        auto data = BlockManager::data(id);
        if (data.sound.walk) {
            fs::path path = data.sound.walk->full_path();
            auto& audio = m_world.get_audio();
            audio.play_3d(path, pos, true);
            Logger::debug("Player block {} walk sound", path.string());
        }
    });

    for (int i = 0; i < 10; i++) {
        m_hotbar[i].id = i;
    }
}

void LocalPlayer::update_speed(float dt) {
    // calculate speed
    auto& v = m_velocity;
    if (m_move_state.forward || m_move_state.back || m_move_state.left ||
        m_move_state.right) {
        m_direction.value = glm::vec3(0.0f, 0.0f, 0.0f);
        v.value.x += m_movement.acceleration * dt;
        v.value.z += m_movement.acceleration * dt;
        if (v.value.x > v.max.x) {
            v.value.x = v.max.x;
        }
        if (v.value.z > v.max.z) {
            v.value.z = v.max.z;
        }
    } else {
        v.value.x += -m_movement.deceleration * dt;
        v.value.z += -m_movement.deceleration * dt;
        if (v.value.z < 0.0f) {
            v.value.z = 0.0f;
        }
        if (v.value.x < 0.0f) {
            v.value.x = 0.0f;
        }
        if (v.value.z < 0.0f && v.value.x < 0.0f) {
            m_direction.value = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    if (m_move_state.is_fly) {
        if (m_move_state.up) {
            v.value.y = v.max.y;
        }

        if (m_move_state.down) {
            v.value.y = -v.max.y;
        }

        if (!m_move_state.down && !m_move_state.up) {
            v.value.y = 0.0f;
        }
    } else {
        if (m_move_state.up && m_move_state.can_up) {
            v.value.y = m_movement.jump_power;
            m_move_state.can_up = false;
        }

        v.value.y += -m_gravity.value * dt;
    }
}

std::tuple<bool, bool, bool> LocalPlayer::update_physical(float dt,
                                                          glm::vec3& pos) {

    auto distance = get_move_distance(dt);
    auto box = HitboxManager::hitbox(m_hitbox);
    bool x = false;
    bool y = false;
    bool z = false;
    if (update_x(pos, distance, m_world, box.box)) {
        pos.x += distance.x;
        x = true;
    } else {
        m_velocity.value.x = 0.0f;
    }

    if (update_y(pos, distance, m_world, box.box)) {
        pos.y += distance.y;
        y = true;
    } else {
        m_velocity.value.y = 0.0f;
        if (distance.y < 0) {
            m_move_state.can_up = true;
            m_move_state.is_fly = false;
        }
    }

    if (update_z(pos, distance, m_world, box.box)) {
        pos.z += distance.z;
        z = true;
    } else {
        m_velocity.value.z = 0.0f;
    }
    return {x, y, z};
}

glm::vec3 LocalPlayer::get_move_distance(float dt) {
    const auto& d = m_direction;
    const auto& v = m_velocity;
    return glm::vec3{d.value.x * v.value.x * dt, v.value.y * dt,
                     d.value.z * v.value.z * dt};
}

bool LocalPlayer::is_underwater() const { return m_underwater; }
void LocalPlayer::set_underwater(bool u) { m_underwater = u; }

glm::vec3& LocalPlayer::max_speed() { return m_velocity.max; }
float& LocalPlayer::acceleration() { return m_movement.acceleration; }
float& LocalPlayer::deceleration() { return m_movement.deceleration; }
float& LocalPlayer::g() { return m_gravity.value; }
void LocalPlayer::set_gait(Gait gait) { m_walk_pose.gait = gait; }
float LocalPlayer::yaw() const { return m_angle.yaw; }
float LocalPlayer::pitch() const { return m_angle.pitch; }
float& LocalPlayer::roll() { return m_angle.roll; }
float& LocalPlayer::walk_time() { return m_walk_pose.walk_time; }
Gait LocalPlayer::get_gait() const { return m_walk_pose.gait; }

} // namespace Cubed