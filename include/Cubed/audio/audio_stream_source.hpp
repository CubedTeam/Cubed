#pragma once

#include "Cubed/audio/audio_effect_slot.hpp"
#include "Cubed/audio/audio_filter.hpp"
#include "glm/ext/vector_float3.hpp"

#include <al.h>
#include <cstdint>
#include <span>
#include <vector>
namespace Cubed {
class AudioStreamSource {
public:
    static constexpr int NUM_BUFFERS = 4;
    AudioStreamSource();
    AudioStreamSource(const AudioStreamSource&) = delete;
    AudioStreamSource(AudioStreamSource&&) = delete;
    AudioStreamSource& operator=(const AudioStreamSource&) = delete;
    AudioStreamSource& operator=(AudioStreamSource&&) = delete;
    ~AudioStreamSource();

    void push_pcm(std::span<const int16_t> pcm, ALsizei sample_rate);

    void stop();
    bool is_playing() const;
    void reset();

    void set_volume(float volume);
    void set_pitch(float pitch);
    void set_pos(glm::vec3 pos);
    void set_filter(const AudioFilter& filter);
    void clear_filter();
    void set_effect_slot(const AudioEffectSlot& slot);
    void clear_effect_slot();

private:
    ALuint m_source = 0;
    std::array<ALuint, NUM_BUFFERS> m_buffers{};
    std::vector<ALuint> m_free_buffers;
    bool m_playing = false;

    void unqueue_processed();
};
} // namespace Cubed