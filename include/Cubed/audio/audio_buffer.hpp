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

private:
    ALuint m_buffer = 0;
    void set_data(const AudioData& data);
};
} // namespace Cubed