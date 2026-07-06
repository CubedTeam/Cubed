#pragma once
#include "Cubed/audio/audio_buffer.hpp"

#include <string>
#include <unordered_map>
namespace Cubed {
class SoundManager {
public:
    SoundManager();
    ~SoundManager();
    void load(const std::string& name);
    const AudioBuffer& get_buffer(const std::string& name) const;
    void init();
    void clear();

private:
    std::unordered_map<std::string, AudioBuffer> m_buffers;
};
} // namespace Cubed