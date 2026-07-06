#include "Cubed/audio/audio_engine.hpp"

#include "Cubed/tools/log.hpp"

#include <stdexcept>
namespace Cubed {
AudioEngine::AudioEngine() {};

AudioEngine::~AudioEngine() {
    if (!m_init) {
        return;
    }

    m_bgm.reset();
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
    m_bgm->set_buffer(m_sounds.get_buffer("bgm/bgm001.mp3"));
    Logger::info("Audio Engine Init Success");

    m_init = true;
}

void AudioEngine::play_bgm() { m_bgm->play(); }

void AudioEngine::update(glm::vec3 listener_pos) {}

} // namespace Cubed