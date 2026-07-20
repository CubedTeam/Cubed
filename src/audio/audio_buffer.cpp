#include "Cubed/audio/audio_buffer.hpp"

#include "Cubed/audio/audio_error.hpp"
namespace Cubed {
AudioBuffer::AudioBuffer(const AudioData& data) {
    alGenBuffers(1, &m_buffer);
    set_data(data);
}
AudioBuffer::~AudioBuffer() { alDeleteBuffers(1, &m_buffer); }

ALuint AudioBuffer::buffer() const { return m_buffer; }
float AudioBuffer::duration() const { return m_duration; }
uint32_t AudioBuffer::channels() const { return m_channels; }

void AudioBuffer::set_data(const AudioData& data) {

    ALenum format;
    if (data.channels == 1) {
        format = AL_FORMAT_MONO16;
    } else {
        format = AL_FORMAT_STEREO16;
    }
    m_channels = data.channels;

    Logger::debug("channels={} rate={} samples={} bytes={} format={}",
                  data.channels, data.sample_rate, data.pcm.size(),
                  data.pcm.size() * sizeof(int16_t), format);
    alBufferData(m_buffer, format, data.pcm.data(),
                 static_cast<ALsizei>(data.pcm.size() * sizeof(int16_t)),
                 static_cast<ALsizei>(data.sample_rate));
    check_al_error();
    ALint size = 0;
    alGetBufferi(m_buffer, AL_SIZE, &size);
    check_al_error();

    Logger::debug("buffer size={}", size);
    m_duration = static_cast<float>(data.pcm.size()) /
                 (data.channels * data.sample_rate);
}

} // namespace Cubed