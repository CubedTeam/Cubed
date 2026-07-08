#pragma once
#include <AL/al.h>
namespace Cubed {
class AudioFilter {
public:
    AudioFilter();
    AudioFilter(const AudioFilter&) = delete;
    AudioFilter(AudioFilter&&) = delete;
    AudioFilter& operator=(const AudioFilter&) = delete;
    AudioFilter& operator=(AudioFilter&&) = delete;
    ~AudioFilter();

    void set_lowpass(float gain, float gain_hf);
    ALuint filter() const;

private:
    ALuint m_filter = 0;
};
} // namespace Cubed