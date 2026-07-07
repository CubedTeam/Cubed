#include "Cubed/audio/audio_source.hpp"

#include "Cubed/audio/audio_error.hpp"
#include "Cubed/tools/log.hpp"

#include <algorithm>
#include <stdexcept>

namespace Cubed {
AudioSource::AudioSource(float volume) {
    alGenSources(1, &m_source);
    set_target_volume(volume);
}
AudioSource::~AudioSource() {
    if (m_source != 0) {
        stop();
        alDeleteSources(1, &m_source);
    }
}

void AudioSource::set_buffer_2d(const AudioBuffer& buffer) {
    if (state() != AudioState::STOPPED && state() != AudioState::INITIAL) {
        stop();
    }
    m_duration = buffer.duration();
    alSourcei(m_source, AL_BUFFER, buffer.buffer());
    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_source, AL_POSITION, 0, 0, 0);
}

void AudioSource::set_buffer_3d(const AudioBuffer& buffer,
                                const glm::vec3& pos) {

    if (buffer.channels() != 1) {
        Logger::warn("3D sound should use mono audio.");
    }

    if (state() != AudioState::STOPPED && state() != AudioState::INITIAL) {
        stop();
    }
    m_duration = buffer.duration();
    alSourcei(m_source, AL_BUFFER, buffer.buffer());
    check_al_error();
    alSource3f(m_source, AL_POSITION, pos.x, pos.y, pos.z);
    check_al_error();

    alSourcef(m_source, AL_REFERENCE_DISTANCE, 4.0f);
    alSourcef(m_source, AL_ROLLOFF_FACTOR, 1.0f);
    alSourcef(m_source, AL_MAX_DISTANCE, 48.0f);
    ALfloat l[3];
    alGetListenerfv(AL_POSITION, l);

    Logger::info("Listener Pos = ({}, {}, {})", l[0], l[1], l[2]);
    ALfloat p[3];
    alGetSourcefv(m_source, AL_POSITION, p);

    Logger::info("Source Pos = ({}, {}, {})", p[0], p[1], p[2]);
}

void AudioSource::set_loop(bool on) {
    if (on) {
        alSourcei(m_source, AL_LOOPING, AL_TRUE);
    } else {
        alSourcei(m_source, AL_LOOPING, AL_FALSE);
    }
}

void AudioSource::set_volume(float volume) {
    volume = std::clamp(volume, 0.0f, 1.0f);
    alSourcef(m_source, AL_GAIN, volume);
}

void AudioSource::play() { alSourcePlay(m_source); }

void AudioSource::play_2d(const AudioBuffer& buffer) {
    set_buffer_2d(buffer);
    play();
}

void AudioSource::play_3d(const AudioBuffer& buffer, const glm::vec3& pos) {
    set_buffer_3d(buffer, pos);
    play();
}

void AudioSource::stop() { alSourceStop(m_source); }
void AudioSource::pause() { alSourcePause(m_source); }

float AudioSource::duration() const { return m_duration; }
float AudioSource::current_time() const {
    ALfloat sec = 0.0f;
    alGetSourcef(m_source, AL_SEC_OFFSET, &sec);
    return static_cast<float>(sec);
}

float AudioSource::target_volume() const { return m_target_volume; }
float& AudioSource::target_volume() { return m_target_volume; }

void AudioSource::set_target_volume(float volume) {
    m_target_volume = volume;
    set_volume(m_target_volume);
}

AudioState AudioSource::state() const {
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
    default:
        throw std::runtime_error("Invalid OpenAL source state");
    }
    throw std::runtime_error("Invaild state");
}

void AudioSource::mark_in_use() { m_using = true; }
bool AudioSource::in_use() const { return m_using; }
void AudioSource::reset() {
    alSourceStop(m_source);

    alSourcei(m_source, AL_BUFFER, 0);

    alSourcef(m_source, AL_GAIN, 1.0f);
    alSourcef(m_source, AL_PITCH, 1.0f);

    alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(m_source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSource3f(m_source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);

    alSourcef(m_source, AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(m_source, AL_ROLLOFF_FACTOR, 1.0f);
    alSourcef(m_source, AL_MAX_DISTANCE, FLT_MAX);

    alSourcef(m_source, AL_CONE_INNER_ANGLE, 360.0f);
    alSourcef(m_source, AL_CONE_OUTER_ANGLE, 360.0f);
    alSourcef(m_source, AL_CONE_OUTER_GAIN, 0.0f);

    alSourcei(m_source, AL_LOOPING, AL_FALSE);

    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_FALSE);

    alSourcef(m_source, AL_SEC_OFFSET, 0.0f);

    m_duration = 0.0f;
    m_target_volume = 1.0f;
    m_using = false;
}
} // namespace Cubed