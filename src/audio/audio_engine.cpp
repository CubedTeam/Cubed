#include "Cubed/audio/audio_engine.hpp"

#include "Cubed/audio/audio_error.hpp"
#include "Cubed/config.hpp"
#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <stdexcept>
namespace Cubed {
AudioEngine::AudioEngine() {};

AudioEngine::~AudioEngine() {
    if (!m_init) {
        return;
    }

    m_bgm.reset();
    m_pool.reset();
    m_sounds.clear();

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void AudioEngine::init() {
    device = alcOpenDevice(NULL);
    if (!device) {
        throw std::runtime_error("Failed to open OpenAL device.");
    }
    context = alcCreateContext(device, nullptr);

    if (!context) {
        throw std::runtime_error("Failed to create OpenAL context.");
    }
    alcMakeContextCurrent(context);

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    check_al_error();

    auto& config = Config::get();

    m_music_volume = static_cast<float>(config.get<double>("volume.music"));
    m_sfx_volume = static_cast<float>(config.get<double>("volume.SFX"));

    m_sounds.init();

    m_bgm = std::make_unique<AudioSource>(m_music_volume);

    m_bgm->set_buffer_2d(m_sounds.get_buffer("bgm/bgm001.mp3"));

    m_fade_map.try_emplace("bgm", m_bgm.get(), 5.0f, 2.0f);

    ALCint max_mono = 0;

    alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &max_mono);

    if (max_mono <= 1) {
        Logger::error("Can't get max mono");
        max_mono = 4;
    }

    Logger::info("Set Source Pool Size {}", static_cast<int>(max_mono));
    // Reserve a source for BGM
    m_pool = std::make_shared<SourcePool>(max_mono - 1);

    Logger::info("Audio Engine Init Success");

    m_init = true;
}

void AudioEngine::play_bgm() { m_bgm->play(); }

void AudioEngine::change_bgm(const std::string& sound) {
    Logger::info("change bgm {}", sound);
    m_bgm->stop();
    m_bgm->reset();
    m_bgm->set_buffer_2d(m_sounds.get_buffer(sound));
    m_bgm->set_target_volume(m_music_volume);
    m_bgm->set_volume(m_music_volume);
    auto it = m_fade_map.find("bgm");
    if (it != m_fade_map.end()) {
        it->second.reset();
    }
    m_bgm->play();
};

void AudioEngine::play_3d(const std::string& sound, const glm::vec3& pos,
                          bool check) {
    if (!m_pool) {
        Logger::error("Source Pool is nullptr");
        return;
    }
    auto* source = m_pool->acquire();
    source->set_volume(m_sfx_volume);
    if (!source) {
        Logger::error("Source is Full");
    }

    try {
        auto& buffer = m_sounds.get_buffer(sound);
        source->play_3d(buffer, pos);
    } catch (const std::exception& e) {
        if (check) {
            ASSERT_MSG(false, e.what());
            Logger::error("Player Sound Error {}", e.what());
        }
    }
}

void AudioEngine::play_2d(const std::string& sound, bool check) {
    if (!m_pool) {
        Logger::error("Source Pool is nullptr");
        return;
    }
    auto* source = m_pool->acquire();
    source->set_volume(m_sfx_volume);
    if (!source) {
        Logger::error("Source is Full");
    }

    try {
        auto& buffer = m_sounds.get_buffer(sound);
        source->play_2d(buffer);
    } catch (const std::exception& e) {
        if (check) {
            ASSERT_MSG(false, e.what());
            Logger::error("Player Sound Error {}", e.what());
        }
    }
}

void AudioEngine::update_listener(const glm::vec3& pos,
                                  const glm::vec3& forward,
                                  const glm::vec3& up) {
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);

    float orientation[] = {forward.x, forward.y, forward.z,

                           up.x,      up.y,      up.z};

    alListenerfv(AL_ORIENTATION, orientation);
}
void AudioEngine::update() {

    for (auto& [key, fade] : m_fade_map) {
        fade.update();
    }
    m_pool->update();
}

void AudioEngine::reload_config() {
    auto& config = Config::get();

    m_music_volume = static_cast<float>(config.get<double>("volume.music"));
    m_sfx_volume = static_cast<float>(config.get<double>("volume.SFX"));
    if (m_bgm) {
        m_bgm->set_target_volume(m_music_volume);
    }
}

float& AudioEngine::bgm_target_volume() { return m_bgm->target_volume(); }

} // namespace Cubed