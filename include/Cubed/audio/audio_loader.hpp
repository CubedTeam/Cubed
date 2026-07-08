#pragma once
#include "Cubed/audio/audio_data.hpp"

#include <filesystem>
namespace Cubed {

class AudioLoader {
public:
    static AudioData load(const std::filesystem::path& path);

private:
    static AudioData load_wav(const std::filesystem::path& path);
    static AudioData load_mp3(const std::filesystem::path& path);
    static AudioData load_flac(const std::filesystem::path& path);
    static AudioData load_ogg(const std::filesystem::path& path);
};
} // namespace Cubed
