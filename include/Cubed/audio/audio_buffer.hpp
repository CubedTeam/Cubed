#pragma once
#include "Cubed/audio/audio_data.hpp"

#include <AL/al.h>
namespace Cubed {
class AudioBuffer {
public:
    AudioBuffer(const AudioData& data);
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer(AudioBuffer&&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;
    AudioBuffer& operator=(AudioBuffer&&) = delete;
    ~AudioBuffer();
    ALuint buffer() const;
    float duration() const;
    uint32_t channels() const;

private:
    ALuint m_buffer = 0;
    float m_duration = 0.0f;
    uint32_t m_channels = 0;
    void set_data(const AudioData& data);
};
} // namespace Cubed