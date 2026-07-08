#include "Cubed/audio/audio_fade.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
namespace Cubed {
AudioFade::AudioFade(AudioSource* source, float fade_in, float fade_out)
    : m_source(source), m_in_duration(fade_in), m_out_duration(fade_out) {
    if (!m_source) {
        m_active = false;
        return;
    }

    m_start_gain = 0.0f;

    m_source->set_volume(0.0f);
}
AudioFade::~AudioFade() {}

void AudioFade::reset() {
    m_active = true;
    m_fade_in = true;
    m_start_gain = 0.0f;
    m_source->set_volume(0.0f);
}

void AudioFade::update() {
    if (!m_active || !m_source) {
        return;
    }

    if (m_fade_in) {
        float t =
            std::clamp(m_source->current_time() / m_in_duration, 0.0f, 1.0f);

        // Ease Out Sine
        t = std::sin(t * std::numbers::pi_v<float> * 0.5f);

        float gain = std::lerp(m_start_gain, m_source->target_volume(), t);
        m_source->set_volume(gain);
        if (t >= 1.0f) {
            m_source->set_volume(m_source->target_volume());
            m_fade_in = false;
            m_start_gain = m_source->target_volume();
        }
    } else {
        float start_time =
            std::max(0.0f, m_source->duration() - m_out_duration);

        if (m_source->current_time() < start_time) {
            return;
        }

        float elapsed = m_source->current_time() - start_time;

        float t = std::clamp(elapsed / m_out_duration, 0.0f, 1.0f);

        // Smoothstep
        t = t * t * (3.0f - 2.0f * t);

        float gain = std::lerp(m_start_gain, 0.0f, t);
        m_source->set_volume(gain);

        if (t >= 1.0f) {
            m_source->set_volume(0.0f);
            m_active = false;
        }
    }
}
} // namespace Cubed