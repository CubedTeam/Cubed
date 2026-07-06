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
    m_bgm->set_buffer_2d(m_sounds.get_buffer("bgm/bgm001.mp3"));

    m_fade_map.try_emplace("bgm", m_bgm.get(), 5.0f, 2.0f);

    Logger::info("Audio Engine Init Success");

    m_init = true;
}

void AudioEngine::play_bgm() { m_bgm->play(); }

void AudioEngine::update_listener(glm::vec3 listener_pos) {}
void AudioEngine::update(float dt) {

    for (auto& [key, fade] : m_fade_map) {
        fade.update();
    }
}
} // namespace Cubed