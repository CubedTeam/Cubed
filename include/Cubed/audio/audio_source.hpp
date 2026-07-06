#pragma once
#include "Cubed/audio/audio_buffer.hpp"

#include <AL/al.h>
namespace Cubed {

enum class AudioState { INITIAL, PLAYING, PAUSED, STOPPED };

class AudioSource {
public:
    AudioSource();
    ~AudioSource();

    void set_buffer(const AudioBuffer& buffer);
    void set_loop(bool on = true);

    void play();
    void stop();
    void pause();

    AudioState state();

private:
    ALuint m_source = 0;
};
} // namespace Cubed
