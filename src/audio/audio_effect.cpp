#include "Cubed/audio/audio_effect.hpp"
#ifndef AL_ALEXT_PROTOTYPES
#define AL_ALEXT_PROTOTYPES
#endif
#include <AL/efx.h>

namespace Cubed {
AudioEffect::AudioEffect() { alGenEffects(1, &m_effect); }

AudioEffect::~AudioEffect() {
    if (m_effect) {
        alDeleteEffects(1, &m_effect);
    }
}

void AudioEffect::set_reverb(float decay_time, float reverb_gain, float gain_hf,
                             float density, float diffusion) {
    alEffecti(m_effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(m_effect, AL_REVERB_DECAY_TIME, decay_time);
    alEffectf(m_effect, AL_REVERB_GAIN, reverb_gain);
    alEffectf(m_effect, AL_REVERB_GAINHF, gain_hf);
    alEffectf(m_effect, AL_REVERB_DENSITY, density);
    alEffectf(m_effect, AL_REVERB_DIFFUSION, diffusion);
}
ALuint AudioEffect::effect() const { return m_effect; }
} // namespace Cubed