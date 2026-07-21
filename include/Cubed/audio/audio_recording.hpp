#pragma once

#include <alc.h>
#include <array>
#include <cstdint>
namespace Cubed {
class AudioEngine;
class AudioRecording {
public:
    static constexpr int SAMPLE_RATE = 48000;
    static constexpr int FRAME_MS = 20;
    static constexpr int FRAME_SAMPLES = SAMPLE_RATE * FRAME_MS / 1000;
    AudioRecording(AudioEngine& engine);
    AudioRecording(const AudioRecording&) = delete;
    AudioRecording(AudioRecording&&) = delete;
    AudioRecording& operator=(const AudioRecording&) = delete;
    AudioRecording& operator=(AudioRecording&&) = delete;
    ~AudioRecording();

    void update();

    void init();
    void start();
    void stop();
    bool is_recording() const;

private:
    AudioEngine& m_engine;
    ALCdevice* m_capture = nullptr;
    bool m_recording = false;
    void send_voice(const std::array<int16_t, FRAME_SAMPLES>& pcm);
};
} // namespace Cubed