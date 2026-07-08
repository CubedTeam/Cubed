#pragma once

#include "Cubed/audio/audio_fade.hpp"
#include "Cubed/audio/audio_source.hpp"
#include "Cubed/audio/sound_manager.hpp"
#include "Cubed/audio/source_pool.hpp"

#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
namespace Cubed {

class ClientWorld;

class AudioEngine {
public:
    AudioEngine();
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine& operator=(AudioEngine&&) = delete;
    ~AudioEngine();

    void init();
    void play_bgm();
    void change_bgm(const std::string& sound);
    void play_3d(const std::string& sound, const glm::vec3& pos,
                 bool check = false);
    void play_2d(const std::string& sound, bool check = false);
    void update_listener(const glm::vec3& pos, const glm::vec3& forward,
                         const glm::vec3& up);
    void update();
    void reload_config();

    float& bgm_target_volume();

private:
    using FadeMap = std::unordered_map<std::string, AudioFade>;
    bool m_init{false};
    ALCdevice* device{nullptr};
    ALCcontext* context{nullptr};
    glm::vec3 listener_pos;
    std::unique_ptr<AudioSource> m_bgm;
    FadeMap m_fade_map;
    SoundManager m_sounds;
    std::shared_ptr<SourcePool> m_pool;

    float m_music_volume = 1.0f;
    float m_sfx_volume = 1.0f;
};
} // namespace Cubed