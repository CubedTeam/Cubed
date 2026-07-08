#pragma once
#include <AL/al.h>
namespace Cubed {
class AudioEffect {
public:
    AudioEffect();
    AudioEffect(const AudioEffect&) = delete;
    AudioEffect(AudioEffect&&) = delete;
    AudioEffect& operator=(const AudioEffect&) = default;
    AudioEffect& operator=(AudioEffect&&) = delete;
    ~AudioEffect();
    void set_reverb(float decay_time, float reverb_gain, float gain_hf,
                    float density = 1.0f, float diffusion = 1.0f);
    ALuint effect() const;

private:
    ALuint m_effect = 0;
};
} // namespace Cubed