#pragma once
#include "Cubed/audio/audio_source.hpp"
namespace Cubed {
class AudioFade {
public:
    AudioFade(AudioSource* source, float fade_in = 0.0f, float fade_out = 0.0f);
    ~AudioFade();
    void update();

private:
    AudioSource* m_source = nullptr;
    float m_in_duration = 0.0f;
    float m_out_duration = 0.0f;
    float m_start_gain = 0.0f;
    float m_end_gain = 0.0f;
    bool m_fade_in = true;
    bool m_active = true;
};
} // namespace Cubed