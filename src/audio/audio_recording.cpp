#include "Cubed/audio/audio_recording.hpp"

#include "Cubed/audio/audio_engine.hpp"
#include "Cubed/tools/log.hpp"

#include <al.h>

namespace Cubed {
AudioRecording::AudioRecording(AudioEngine& engine)
    : m_engine(engine) {

      };
AudioRecording::~AudioRecording() {
    if (m_capture) {
        if (m_recording) {
            stop();
        }

        alcCaptureCloseDevice(m_capture);
    }
}

void AudioRecording::init() {
    m_capture =
        alcCaptureOpenDevice(nullptr, SAMPLE_RATE, AL_FORMAT_MONO16, 4096);
    if (m_capture) {
        Logger::info("Open Audio Capture Success");
    } else {
        Logger::error("Fail to Open Audio Capture Device");
    }
    ALCint freq = 0;
    alcGetIntegerv(m_capture, ALC_FREQUENCY, 1, &freq);

    Logger::info("Capture frequency={}", freq);
}

void AudioRecording::start() {
    if (m_recording) {
        return;
    }
    ALCint samples;

    alcGetIntegerv(m_capture, ALC_CAPTURE_SAMPLES, 1, &samples);

    std::vector<int16_t> dump(samples);
    alcCaptureSamples(m_capture, dump.data(), samples);
    alcCaptureStart(m_capture);
    m_recording = true;
}

void AudioRecording::stop() {
    if (!m_recording) {
        return;
    }
    alcCaptureStop(m_capture);
    m_recording = false;
}
bool AudioRecording::is_recording() const { return m_recording; }
void AudioRecording::update() {
    if (!m_recording || !m_capture) {
        return;
    }

    ALCint available = 0;
    alcGetIntegerv(m_capture, ALC_CAPTURE_SAMPLES, 1, &available);
    while (available >= FRAME_SAMPLES) {
        std::array<int16_t, FRAME_SAMPLES> pcm;
        alcCaptureSamples(m_capture, pcm.data(), FRAME_SAMPLES);
        send_voice(pcm);
        available -= FRAME_SAMPLES;
    }
}
void AudioRecording::send_voice(const std::array<int16_t, FRAME_SAMPLES>& pcm) {

    m_engine.send_voice(pcm);
}
} // namespace Cubed