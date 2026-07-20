#include "Cubed/audio/audio_stream_source.hpp"

#include "Cubed/audio/audio_error.hpp"
#include "Cubed/tools/log.hpp"

#include <cfloat>
#include <efx.h>
namespace Cubed {
AudioStreamSource::AudioStreamSource() {
    alGenBuffers(NUM_BUFFERS, m_buffers.data());
    alGenSources(1, &m_source);

    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    m_free_buffers.assign(m_buffers.begin(), m_buffers.end());
}

AudioStreamSource::~AudioStreamSource() {
    stop();
    alDeleteSources(1, &m_source);
    alDeleteBuffers(NUM_BUFFERS, m_buffers.data());
}

void AudioStreamSource::push_pcm(std::span<const int16_t> pcm,
                                 ALsizei sample_rate) {
    if (!m_source) {
        return;
    }
    unqueue_processed();

    if (m_free_buffers.empty()) {
        Logger::warn("Queue is full, dropping frame");
        return;
    }

    ALuint buf = m_free_buffers.back();
    m_free_buffers.pop_back();

    alBufferData(buf, AL_FORMAT_MONO16, pcm.data(),
                 static_cast<ALsizei>(pcm.size() * sizeof(int16_t)),
                 sample_rate);
    alSourceQueueBuffers(m_source, 1, &buf);

    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        alSourcePlay(m_source);
        m_playing = true;
    }
}

void AudioStreamSource::stop() {
    if (!m_source) {
        return;
    }

    alSourceStop(m_source);

    alSourcei(m_source, AL_BUFFER, 0);
    m_free_buffers.assign(m_buffers.begin(), m_buffers.end());
    m_playing = false;
}

void AudioStreamSource::unqueue_processed() {
    ALint processed = 0;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

    while (processed-- > 0) {
        ALuint buf;
        alSourceUnqueueBuffers(m_source, 1, &buf);
        m_free_buffers.push_back(buf);
    }
}

void AudioStreamSource::set_volume(float volume) {
    volume = std::clamp(volume, 0.0f, 1.0f);
    alSourcef(m_source, AL_GAIN, volume);
}

void AudioStreamSource::set_pitch(float pitch) {
    pitch = std::clamp(pitch, 0.0f, 1.0f);
    alSourcef(m_source, AL_PITCH, pitch);
}

void AudioStreamSource::set_pos(glm::vec3 pos) {
    alSourcei(m_source, AL_SOURCE_RELATIVE, AL_FALSE);
    alSource3f(m_source, AL_POSITION, pos.x, pos.y, pos.z);
    check_al_error();

    alSourcef(m_source, AL_REFERENCE_DISTANCE, 4.0f);
    alSourcef(m_source, AL_ROLLOFF_FACTOR, 1.0f);
    alSourcef(m_source, AL_MAX_DISTANCE, 48.0f);
}

void AudioStreamSource::reset() {
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

    clear_filter();
    clear_effect_slot();
}

void AudioStreamSource::set_filter(const AudioFilter& filter) {
    alSourcei(m_source, AL_DIRECT_FILTER, filter.filter());
}
void AudioStreamSource::clear_filter() {
    alSourcei(m_source, AL_DIRECT_FILTER, AL_FILTER_NULL);
}

void AudioStreamSource::set_effect_slot(const AudioEffectSlot& slot) {
    alSource3i(m_source, AL_AUXILIARY_SEND_FILTER, slot.slot(), 0,
               AL_FILTER_NULL);
}

void AudioStreamSource::clear_effect_slot() {
    alSource3i(m_source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0,
               AL_FILTER_NULL);
}

} // namespace Cubed