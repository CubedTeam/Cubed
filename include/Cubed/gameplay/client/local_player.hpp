#pragma once
#include "Cubed/constants.hpp"
#include "Cubed/crypto/ed25519.hpp"
#include "Cubed/gameplay/block.hpp"
#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/ecs/animation.hpp"
#include "Cubed/gameplay/ecs/identity.hpp"
#include "Cubed/gameplay/ecs/movement.hpp"
#include "Cubed/gameplay/ecs/state.hpp"
#include "Cubed/gameplay/ecs/transform.hpp"
#include "Cubed/gameplay/game_mode.hpp"
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/hitbox.hpp"
#include "Cubed/gameplay/item_stack.hpp"
#include "Cubed/input/event.hpp"
#include "Cubed/input/input.hpp"

#include <absl/container/flat_hash_set.h>
#include <glm/glm.hpp>
#include <optional>
#include <shared_mutex>
namespace cubed {

class ClientWorld;
class LocalPlayer {
public:
    static constexpr float WALK_SOUND_INTERVAL = 0.45f;
    static constexpr float RUN_SOUND_INTERVAL = 0.3f;
    using ChunkPosSet = absl::flat_hash_set<ChunkPos, ChunkPos::Hash>;
    LocalPlayer(ClientWorld& world);
    ~LocalPlayer();

    void clear_key();
    void reset_speed();
    bool handle_mouse_button_event(const MouseButtonEvent& e);
    bool handle_key_event(const KeyEvent& e);
    bool handle_mouse_wheel_event(const MouseWheelEvent& e);

    void update_front_vec(float offset_x, float offset_y);
    bool update_player_move_state(Key key, KeyAction action);
    bool update_scroll(float yoffset);

    void update_chunk_set(const ChunkPosSet& set);

    const ChunkPosSet& get_chunk_pos_set() const;
    ChunkPosSet get_chunk_pos_set();

    const glm::vec3& get_front() const;

    const std::optional<LookBlock>& get_look_block_pos() const;
    // thread safe
    glm::vec3 get_player_pos() const;
    const MoveState& get_move_state() const;

    void change_mode(GameMode mode);
    void reload_config();
    void set_player_pos(const glm::vec3& pos);
    void update(float delta_time);

    float& max_walk_speed();
    float& max_run_speed();
    float& fly_y_speed();

    std::optional<ItemStack> get_current_itemstack() const;

    GameMode& game_mode();

    ClientWorld& get_world();

    Uuid get_uuid() const;
    const std::string& get_name() const;
    void reset_input_status();
    void init(std::string_view name);

    bool ray_cast(const glm::vec3& start, const glm::vec3& dir,
                  glm::ivec3& block_pos, glm::vec3& normal,
                  float distance = 4.0f);
    bool is_underwater() const;
    void set_underwater(bool u);
    void place_block(float dt);

    int selected_hotbar() const;

    void set_inventory(int pos, std::optional<ItemStack> item);

    std::span<const std::optional<ItemStack>> get_inventory() const;

    auto get_hotbar() const {
        return std::span{m_inventory}.first(HOTBAR_SIZE);
    }

    auto get_backpack() const {
        return std::span{m_inventory}.subspan(HOTBAR_SIZE);
    }

    glm::vec3& max_speed();
    float& acceleration();
    float& deceleration();
    float& g();
    void set_gait(Gait gait);
    float yaw() const;
    float pitch() const;
    void set_yaw(float yaw);
    void set_pitch(float pitch);
    float& roll();
    float& walk_time();
    Gait get_gait() const;

    std::optional<crypto::Ed25519KeyPair>& key_pair();

private:
    using enum GameMode;
    float m_max_walk_speed = DEFAULT_MAX_WALK_SPEED;
    float m_max_run_speed = DEFAULT_MAX_RUN_SPEED;
    float m_max_y_speed = 7.5f;
    static constexpr float MAX_SPACE_ON_TIME = 0.3f;
    static constexpr float PLACE_BLOCK_INTERVAL = 0.2f;

    std::optional<crypto::Ed25519KeyPair> m_key_pair;

    EntityInfo m_info;
    Position m_pos;
    WalkPose m_walk_pose;
    Velocity m_velocity;
    Orientation m_angle;
    Movement m_movement;
    Gravity m_gravity;
    MoveState m_move_state;
    Direction m_direction;
    HitboxID m_hitbox = 0;

    float m_place_time = PLACE_BLOCK_INTERVAL;

    std::array<std::optional<ItemStack>, INVENTORY_SIZE> m_inventory;
    float m_sensitivity = 0.15f;

    float space_on_time = 0.0f;
    bool space_on = false;

    int m_selected_hotbar = 0;

    bool m_moving = false;
    bool m_sprinting = false;
    bool m_underwater = false;

    // player is tow block tall, the pos is the lower pos
    ChunkPos m_last_chunk_pos{0, 0};

    glm::vec3 m_front{0, 0, -1};
    glm::vec3 m_right{0, 0, 0};

    MouseState m_mouse_state{};
    GameMode m_game_mode = CREATIVE;
    std::optional<LookBlock> m_look_block = std::nullopt;
    std::string m_name{};
    mutable std::shared_mutex m_uuid_mutex;
    Uuid m_uuid;
    ClientWorld& m_world;

    std::unordered_map<std::string, Timer> m_timers;

    mutable std::shared_mutex m_player_pos_mutex;
    mutable std::shared_mutex m_chunk_pos_mutex;
    ChunkPosSet m_player_chunk_pos_set;

    void init_identity();
    void create_identity(const std::filesystem::path& path);

    void update_direction();
    void update_lookup_block();
    void update_move(float dt);
    void update_player_chunk();

    void play_walk_sound(float dt);
    Gait compute_gait() const;

    void update_speed(float dt);
    std::tuple<bool, bool, bool> update_physical(float dt, glm::vec3& pos);
    glm::vec3 get_move_distance(float dt);
};
} // namespace cubed
