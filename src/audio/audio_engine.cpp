#include "Cubed/audio/audio_engine.hpp"

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

    m_sounds.init();
    m_bgm = std::make_unique<AudioSource>();
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

void AudioEngine::play_3d(const std::string& sound, const glm::vec3& pos,
                          bool check) {
    if (!m_pool) {
        Logger::error("Source Pool is nullptr");
        return;
    }
    auto* source = m_pool->acquire();
    if (!source) {
        Logger::error("Source is Full");
    }

    try {
        auto& buffer = m_sounds.get_buffer(sound);
        source->play_3d(buffer, pos);
    } catch (const std::exception& e) {
        if (check) {
            ASSERT_MSG(false, e.what());
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
void AudioEngine::update(float dt) {

    for (auto& [key, fade] : m_fade_map) {
        fade.update();
    }
    m_pool->update();
}
} // namespace Cubed