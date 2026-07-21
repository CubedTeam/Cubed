#include "Cubed/audio/audio_loader.hpp"

#include "Cubed/tools/log.hpp"

#define STB_VORBIS_IMPLEMENTATION
#include "stb/stb_vorbis.h"

#include <dr_flac.h>
#include <dr_mp3.h>
#include <dr_wav.h>

namespace fs = std::filesystem;
namespace Cubed {
AudioData AudioLoader::load(const std::filesystem::path& path) {
    if (!fs::is_regular_file(path)) {
        std::string err = std::format("Path {} is not a file", path.string());

        throw std::runtime_error(err);
    }
    auto ext = path.extension().string();

    if (ext == ".wav") {
        return load_wav(path);
    }
    if (ext == ".mp3") {
        return load_mp3(path);
    }
    if (ext == ".flac") {
        return load_flac(path);
    }
    if (ext == ".ogg") {
        return load_ogg(path);
    }
    throw std::runtime_error(std::format("Unsupported audio format {}", ext));
}

AudioData AudioLoader::load_wav(const std::filesystem::path& path) {
    drwav wav{};
    if (!drwav_init_file(&wav, path.string().c_str(), nullptr)) {
        throw std::runtime_error("Failed to open wav");
    }
    AudioData data;
    data.channels = wav.channels;
    data.sample_rate = wav.sampleRate;
    data.pcm.resize(wav.totalPCMFrameCount * wav.channels);
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, data.pcm.data());

    drwav_uninit(&wav);
    Logger::debug("{} channels={} rate={} samples={}", path.filename().string(),
                  data.channels, data.sample_rate, data.pcm.size());
    return data;
}

AudioData AudioLoader::load_mp3(const std::filesystem::path& path) {
    drmp3_config config;

    drmp3_uint64 frame_count;

    int16_t* pcm = drmp3_open_file_and_read_pcm_frames_s16(
        path.string().c_str(), &config, &frame_count, nullptr);

    if (!pcm)
        throw std::runtime_error("mp3 load failed");

    AudioData data;

    data.channels = config.channels;
    data.sample_rate = config.sampleRate;

    data.pcm.assign(pcm, pcm + frame_count * config.channels);

    drmp3_free(pcm, nullptr);
    Logger::debug("{} channels={} rate={} samples={}", path.filename().string(),
                  data.channels, data.sample_rate, data.pcm.size());
    return data;
}

AudioData AudioLoader::load_flac(const std::filesystem::path& path) {
    drflac_uint64 frame_count;
    unsigned int channels;
    unsigned int sample_rate;

    int16_t* pcm = drflac_open_file_and_read_pcm_frames_s16(
        path.string().c_str(), &channels, &sample_rate, &frame_count, nullptr);

    if (!pcm)
        throw std::runtime_error("Failed to load flac");

    AudioData data;
    data.channels = channels;
    data.sample_rate = sample_rate;

    data.pcm.assign(pcm, pcm + frame_count * channels);

    drflac_free(pcm, nullptr);
    Logger::debug("{} channels={} rate={} samples={}", path.filename().string(),
                  data.channels, data.sample_rate, data.pcm.size());
    return data;
}

AudioData AudioLoader::load_ogg(const std::filesystem::path& path) {
    int error = 0;
    stb_vorbis* vorbis =
        stb_vorbis_open_filename(path.string().c_str(), &error, nullptr);
    if (!vorbis) {
        throw std::runtime_error("Failed to open Ogg Vorbis file: " +
                                 path.string());
    }

    stb_vorbis_info info = stb_vorbis_get_info(vorbis);
    int channels = info.channels;
    int sample_rate = info.sample_rate;

    int total_frames = stb_vorbis_stream_length_in_samples(vorbis);
    if (total_frames <= 0) {
        stb_vorbis_close(vorbis);
        throw std::runtime_error(
            "Failed to get Ogg stream length (or stream is empty)");
    }

    std::vector<int16_t> pcm(total_frames * channels);

    int frames_read = stb_vorbis_get_samples_short_interleaved(
        vorbis, channels, pcm.data(), pcm.size());

    if (frames_read < total_frames) {
        pcm.resize(frames_read * channels);
    }

    stb_vorbis_close(vorbis);

    AudioData data;
    data.channels = channels;
    data.sample_rate = sample_rate;
    data.pcm = std::move(pcm);

    Logger::debug("{} channels={} rate={} samples={}", path.filename().string(),
                  data.channels, data.sample_rate, data.pcm.size());

    return data;
}

} // namespace Cubed