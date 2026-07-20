#include "Cubed/audio/audio_stream_source.hpp"

#include "Cubed/tools/log.hpp"
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

} // namespace Cubed