#include "Cubed/audio/audio_filter.hpp"
#ifndef AL_ALEXT_PROTOTYPES
#define AL_ALEXT_PROTOTYPES
#endif
#include <AL/alc.h>
#include <AL/efx.h>
namespace Cubed {
AudioFilter::AudioFilter() { alGenFilters(1, &m_filter); }
AudioFilter::~AudioFilter() {
    if (m_filter != 0) {
        alDeleteFilters(1, &m_filter);
    }
}
void AudioFilter::set_lowpass(float gain, float gain_hf) {
    alFilteri(m_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    alFilterf(m_filter, AL_LOWPASS_GAIN, gain);
    alFilterf(m_filter, AL_LOWPASS_GAINHF, gain_hf);
}

ALuint AudioFilter::filter() const { return m_filter; }

} // namespace Cubed