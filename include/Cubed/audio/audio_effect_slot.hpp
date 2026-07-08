#pragma once
#include "Cubed/audio/audio_effect.hpp"
namespace Cubed {
class AudioEffectSlot {
public:
    AudioEffectSlot();
    AudioEffectSlot(const AudioEffectSlot&) = delete;
    AudioEffectSlot(AudioEffectSlot&&) = delete;
    AudioEffectSlot& operator=(const AudioEffectSlot&) = delete;
    AudioEffectSlot& operator=(AudioEffectSlot&&) = delete;
    ~AudioEffectSlot();
    void set_effect(const AudioEffect& effect);
    ALuint slot() const;

private:
    ALuint m_slot = 0;
};
} // namespace Cubed