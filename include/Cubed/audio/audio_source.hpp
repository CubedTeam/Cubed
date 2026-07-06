#pragma once
#include "Cubed/audio/audio_buffer.hpp"

#include <AL/al.h>
#include <glm/glm.hpp>
namespace Cubed {

enum class AudioState { INITIAL, PLAYING, PAUSED, STOPPED };

class AudioSource {
public:
    AudioSource();
    ~AudioSource();

    void set_buffer_2d(const AudioBuffer& buffer);
    void set_buffer_3d(const AudioBuffer& buffer, const glm::vec3& vec3);
    void set_loop(bool on = true);
    void set_volume(float volume);

    void play();
    void play_2d(const AudioBuffer& buffer);
    void play_3d(const AudioBuffer& buffer, const glm::vec3& pos);

    void stop();
    void pause();
    float duration() const;
    float current_time() const;
    float volume() const;
    AudioState state() const;

    void mark_in_use();
    bool in_use() const;
    void reset();

private:
    ALuint m_source = 0;
    float m_volume = 1.0f;
    float m_duration = 0.0f;
    bool m_using;
};
} // namespace Cubed
