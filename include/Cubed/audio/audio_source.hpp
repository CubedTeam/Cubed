#pragma once
#include "Cubed/audio/audio_buffer.hpp"

#include <AL/al.h>
namespace Cubed {

enum class AudioState { INITIAL, PLAYING, PAUSED, STOPPED };

class AudioSource {
public:
    AudioSource();
    ~AudioSource();

    void set_buffer_2d(const AudioBuffer& buffer);
    void set_loop(bool on = true);
    void set_volume(float volume);
    void play();
    void stop();
    void pause();
    float duration() const;
    float current_time() const;
    float volume() const;
    AudioState state() const;

private:
    ALuint m_source = 0;
    float m_volume = 1.0f;
    float m_duration = 0.0f;
};
} // namespace Cubed
