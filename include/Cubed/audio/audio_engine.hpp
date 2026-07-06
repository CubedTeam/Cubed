#pragma once

#include "Cubed/audio/audio_source.hpp"
#include "Cubed/audio/sound_manager.hpp"

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
    void update(glm::vec3 listener_pos);

private:
    using SourcePool = std::unordered_map<std::string, ALuint>;
    bool m_init{false};
    ALCdevice* device{nullptr};
    ALCcontext* context{nullptr};
    glm::vec3 listener_pos;
    std::unique_ptr<AudioSource> m_bgm;
    SoundManager m_sounds;
};
} // namespace Cubed