#pragma once
#include "Cubed/audio/audio_engine.hpp"
#include "Cubed/config.hpp"
#include "Cubed/gameplay/block.hpp"
#include "Cubed/gameplay/chat_message.hpp"
#include "Cubed/gameplay/chunk_pos.hpp"
#include "Cubed/gameplay/client/client_chunk.hpp"
#include "Cubed/gameplay/client/client_entity_manager.hpp"
#include "Cubed/gameplay/client/client_player_manager.hpp"
#include "Cubed/gameplay/client/local_player.hpp"
#include "Cubed/gameplay/client/network_client.hpp"
#include "Cubed/gameplay/game_time.hpp"
#include "Cubed/gameplay/world.hpp"
#include "Cubed/input/event.hpp"
#include "Cubed/tools/cubed_random.hpp"
#include "Cubed/tools/priority_thread_pool.hpp"

#include <absl/container/flat_hash_set.h>
#include <deque>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
namespace Cubed {

class WorldScene;
class ClientWorld : public World {
public:
    ClientWorld(const ClientWorld&) = delete;
    ClientWorld(ClientWorld&&) = delete;
    ClientWorld& operator=(const ClientWorld&) = delete;
    ClientWorld& operator=(ClientWorld&&) = delete;
    ClientWorld(AudioEngine& auido, Config& config, WorldScene& scene);
    ~ClientWorld();
    void init(std::string_view player_name,
              std::shared_ptr<NetworkClient> client, RunMode mode);
    void update(float dt);
    bool handle_event(const Event& e);
    const std::optional<LookBlock>& get_look_block_pos() const;
    LocalPlayer& get_player();
    const LocalPlayer& get_player() const;
    int get_block(const glm::ivec3& block_pos) const override;
    bool is_solid(const glm::ivec3& block_pos) const override;
    bool can_pass_block(const glm::ivec3& block_pos) const override;
    BlockType get_block_tpye(const glm::ivec3& block_pos) const override;

    void rebuild_world();

    void push_delete_vbo(std::unique_ptr<VertexBuffer>& vbo);
    void push_delete_vao(std::unique_ptr<VertexArray>& vao);
    // void hot_reload();

    // void rebuild_world();
    void report_block_change(const glm::ivec3& pos, unsigned id) const;
    void receive_block_change(const BlockChangeRsp& rsp);
    void receive_time(const UpdateTime& rsp);

    void receive_player_logout(const LogoutRsp& rsp);
    void receive_player_water_sound(const PlayerWaterSound& rsp);
    void send_player_water_sound(bool underwater, const glm::vec3& pos);
    int rendering_distance() const;
    void rendering_distance(int rendering_distance);
    int get_chunk_task_id() const;
    void start_client_thread(std::string_view uuid);
    void receive_login_rsp(LoginRsp& rsp);
    void stop_client_thread();

    void start_thread_pool();
    void stop_thread_pool();
    void change_pool_threads(int threads);
    void reload_config(bool chunk_build = true);
    void request_chunk();
    void reset_key_status();
    std::vector<glm::vec4>& planes();
    const std::vector<const ChunkRenderSnapshot*>& render_snapshots() const;

    glm::vec3 sunlight_dir() const;
    bool sphere_collide_world(glm::vec3 center, float radius) const;
    void receive_chunk(std::vector<uint8_t> data, PacketHeader header);
    void request_exit();
    bool is_receive_exit();
    int chunk_size() const;

    AudioEngine& get_audio();
    const AudioEngine& get_audio() const;
    Config& get_config();
    WorldScene& world_scene();
    ClientPlayerManager& player_manager();
    ClientEntityManager& entity_manager();
    std::shared_ptr<NetworkClient> get_client() const;
    void set_direct_exit();

    void receive_chat_message(ChatMsg& msg);
    void send_chat_message(ChatMessage& message);
    void receive_voice_message(VoiceMsg& msg);
    bool enable_voice_chat() const;
    int get_per_tick_time() const override;

    bool is_render(const glm::vec3& pos) const;

    template <typename Fn>
    void register_timer(std::string_view id, float threshold, Fn&& f) {
        m_timers.emplace(std::piecewise_construct,
                         std::forward_as_tuple(std::string(id)),
                         std::forward_as_tuple(threshold, std::forward<Fn>(f)));
    }

private:
    struct VoiceMessage {
        std::string data;
        glm::vec3 pos{0.0f};
    };

    std::atomic<bool> m_is_pending_delete_queue_free{false};
    std::mutex m_delete_vbo_mutex;
    std::mutex m_delete_vao_mutex;
    std::vector<std::unique_ptr<VertexBuffer>> m_pending_delete_vbo;
    std::vector<std::unique_ptr<VertexArray>> m_pending_delete_vao;

    enum class ChunkLoadStyle { RANDOM, CENTER };
    using ChunkHashMap =
        tbb::concurrent_hash_map<ChunkPos, std::shared_ptr<ClientChunk>,
                                 ChunkPos::TBBHash>;
    using ChunkPosSet = absl::flat_hash_set<ChunkPos, ChunkPos::Hash>;
    using ChunkPosVector = std::vector<ChunkPos>;
    using chunk_acc = ChunkHashMap::accessor;
    using chunk_cacc = ChunkHashMap::const_accessor;

    struct PendingSound {
        std::string sound;
        glm::vec3 sound_pos{0.0f};
    };

    static constexpr int WORLD_EXIT_TIMEOUT = 200;
    static constexpr int MAX_UPLOAD_CHUNK_SUM = 16;
    std::atomic<RunMode> m_runmode = RunMode::HYBRID;
    ClientEntityManager m_entity_manager;
    ClientPlayerManager m_player_manager;
    ChunkHashMap m_chunks;
    AudioEngine& m_audio;
    Config& m_config;
    WorldScene& m_world_scene;
    std::vector<glm::vec4> m_planes;

    tbb::concurrent_queue<std::unique_ptr<ClientChunk>> m_pending_upload_queue;
    tbb::concurrent_queue<ChunkPos> m_dirty_chunk_queue;
    tbb::concurrent_queue<PendingSound> m_pending_sound;
    tbb::concurrent_queue<ChatMessage> m_message_queue;
    tbb::concurrent_queue<VoiceMessage> m_voice_queue;

    std::deque<ChunkPos> m_dirty_queue;
    std::vector<const ChunkRenderSnapshot*> m_render_snapshots;

    std::unordered_map<std::string, Timer> m_timers;
    std::atomic<bool> m_exit_direct{false};
    std::atomic<bool> m_game_running{false};
    std::atomic<bool> m_receive_exit{false};
    std::atomic<int> m_rendering_distance{24};
    std::atomic<TickType> m_game_ticks{0};
    std::atomic<TickType> m_day_tick{6000};
    std::atomic<int> m_per_tick_time = DEFAULT_PER_TICK_TIME;
    std::atomic<bool> m_requesting_chunk{false};
    std::atomic<bool> m_is_rebuilding{false};
    std::atomic<int> m_chunk_task_id{0};
    std::atomic<bool> m_voice_chat{true};
    std::shared_ptr<NetworkClient> m_client;
    ChunkLoadStyle m_chunk_load_style{ChunkLoadStyle::CENTER};

    std::atomic<std::shared_ptr<PriorityThreadPool>> m_thread_pool;

    Random m_random;

    void set_block(const glm::ivec3& pos, unsigned id);

    void update_chunk(const ChunkPosSet& old, const ChunkPosSet& now);
};
} // namespace Cubed
