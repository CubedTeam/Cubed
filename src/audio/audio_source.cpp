#include "Cubed/audio/audio_source.hpp"

#include <stdexcept>

namespace Cubed {
AudioSource::AudioSource() { alGenSources(1, &m_source); }
AudioSource::~AudioSource() {
    if (m_source != 0) {
        stop();
        alDeleteSources(1, &m_source);
    }
}

void AudioSource::set_buffer(const AudioBuffer& buffer) {
    if (state() == AudioState::PLAYING) {
        stop();
    }
    alSourcei(m_source, AL_BUFFER, buffer.buffer());
}

void AudioSource::set_loop(bool on) {
    if (on) {
        alSourcei(m_source, AL_LOOPING, AL_TRUE);
    } else {
        alSourcei(m_source, AL_LOOPING, AL_FALSE);
    }
}

void AudioSource::play() { alSourcePlay(m_source); }

void AudioSource::stop() { alSourceStop(m_source); }
void AudioSource::pause() { alSourcePause(m_source); }

AudioState AudioSource::state() {
    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    switch (state) {
    case AL_INITIAL:
        return AudioState::INITIAL;
    case AL_PLAYING:
        return AudioState::PLAYING;
    case AL_STOPPED:
        return AudioState::STOPPED;
    case AL_PAUSED:
        return AudioState::PAUSED;
    }
    throw std::runtime_error("Invaild state");
}

} // namespace Cubed