#include "Cubed/audio/audio_effect_slot.hpp"
#ifndef AL_ALEXT_PROTOTYPES
#define AL_ALEXT_PROTOTYPES
#endif
#include <AL/efx.h>
namespace Cubed {
AudioEffectSlot::AudioEffectSlot() { alGenAuxiliaryEffectSlots(1, &m_slot); }

AudioEffectSlot::~AudioEffectSlot() {
    if (m_slot) {
        alDeleteAuxiliaryEffectSlots(1, &m_slot);
    }
}

void AudioEffectSlot::set_effect(const AudioEffect& effect) {
    alAuxiliaryEffectSloti(m_slot, AL_EFFECTSLOT_EFFECT, effect.effect());
}
ALuint AudioEffectSlot::slot() const { return m_slot; }
} // namespace Cubed