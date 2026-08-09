#pragma once
#include "Cubed/audio/audio_buffer.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
namespace Cubed {
class SoundManager {
public:
    SoundManager();
    ~SoundManager();
    const AudioBuffer& load(const std::filesystem::path& path, bool full_path);
    const AudioBuffer& get_buffer(const std::string& name, bool full_path);
    void init();
    void clear();

private:
    std::unordered_map<std::string, AudioBuffer> m_buffers;
};
} // namespace Cubed