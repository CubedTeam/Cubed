#pragma once
#include "Cubed/audio/audio_buffer.hpp"
#include "Cubed/audio/audio_filter.hpp"

#include <AL/al.h>
#include <glm/glm.hpp>
namespace Cubed {

enum class AudioState { INITIAL, PLAYING, PAUSED, STOPPED };

class AudioSource {
public:
    AudioSource(float volume = 1.0f);
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
    float target_volume() const;
    float& target_volume();
    void set_target_volume(float volume);
    AudioState state() const;

    void mark_in_use();
    bool in_use() const;
    void reset();

    void set_filter(const AudioFilter& filter);
    void clear_filter();

private:
    ALuint m_source = 0;
    float m_target_volume = 1.0f;
    float m_duration = 0.0f;
    bool m_using;
};
} // namespace Cubed
