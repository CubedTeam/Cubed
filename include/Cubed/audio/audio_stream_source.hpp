#pragma once

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

private:
    ALuint m_source = 0;
    std::array<ALuint, NUM_BUFFERS> m_buffers{};
    std::vector<ALuint> m_free_buffers;
    bool m_playing = false;

    void unqueue_processed();
};
} // namespace Cubed