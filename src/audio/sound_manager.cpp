#include "Cubed/audio/sound_manager.hpp"

#include "Cubed/audio/audio_loader.hpp"
#include "Cubed/tools/log.hpp"

#include <filesystem>
namespace fs = std::filesystem;
namespace Cubed {
SoundManager::SoundManager() {}
SoundManager::~SoundManager() { clear(); }
void SoundManager::clear() { m_buffers.clear(); }
void SoundManager::init() {
    try {
        load("bgm/bgm001.mp3");
        load("ambient/birds.ogg");
    } catch (const std::exception& e) {
    }
}

const AudioBuffer& SoundManager::load(const std::string& name) {
    fs::path sound_path{fs::path(ASSETS_PATH) / "sound" / name};
    try {

        AudioData data = AudioLoader::load(sound_path);
        auto [pos, inserted] = m_buffers.try_emplace(name, data);
        if (!inserted) {
            Logger::error("Key Already exist, check the sound name {}", name);
        }
        return pos->second;
    } catch (const std::exception& e) {
        Logger::error("Load Sound Error {}", e.what());
        throw;
    }
}
const AudioBuffer& SoundManager::get_buffer(const std::string& name) {
    auto it = m_buffers.find(name);
    if (it == m_buffers.end()) {
        try {
            return load(name);
        } catch (const std::exception& e) {
            std::string err = std::format("Can't Find Buffer {}", name);

            throw std::runtime_error(err);
        }
    }
    return it->second;
}
} // namespace Cubed