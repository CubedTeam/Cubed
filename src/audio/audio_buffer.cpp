#include "Cubed/audio/audio_buffer.hpp"

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
    alBufferData(m_buffer, format, data.pcm.data(), data.pcm.size(),
                 data.sample_rate);
    m_duration = static_cast<float>(data.pcm.size()) /
                 (data.channels * data.sample_rate);
}

} // namespace Cubed