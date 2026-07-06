#include "Cubed/audio/audio_source.hpp"

#include "Cubed/tools/log.hpp"

#include <stdexcept>

namespace Cubed {
AudioSource::AudioSource() { alGenSources(1, &m_source); }
AudioSource::~AudioSource() {
    if (m_source != 0) {
        stop();
        alDeleteSources(1, &m_source);
    }
}

void AudioSource::set_buffer_2d(const AudioBuffer& buffer) {
    if (state() == AudioState::PLAYING) {
        stop();
    }
    m_duration = buffer.duration();
    alSourcei(m_source, AL_BUFFER, buffer.buffer());
    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_source, AL_POSITION, 0, 0, 0);
}

void AudioSource::set_loop(bool on) {
    if (on) {
        alSourcei(m_source, AL_LOOPING, AL_TRUE);
    } else {
        alSourcei(m_source, AL_LOOPING, AL_FALSE);
    }
}

void AudioSource::set_volume(float volume) {
    if (volume > 1.0f) {
        Logger::error("Volume {} is too large", volume);
        return;
    }
    m_volume = volume;
    alSourcef(m_source, AL_GAIN, volume);
}

void AudioSource::play() { alSourcePlay(m_source); }

void AudioSource::stop() { alSourceStop(m_source); }
void AudioSource::pause() { alSourcePause(m_source); }

float AudioSource::duration() const { return m_duration; }
float AudioSource::current_time() const {
    ALfloat sec = 0.0f;
    alGetSourcef(m_source, AL_SEC_OFFSET, &sec);
    return static_cast<float>(sec);
}
float AudioSource::volume() const { return m_volume; }

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