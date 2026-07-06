#include "Cubed/audio/sound_manager.hpp"

#include "Cubed/audio/audio_loader.hpp"
#include "Cubed/tools/cubed_assert.hpp"

#include <filesystem>
namespace fs = std::filesystem;
namespace Cubed {
SoundManager::SoundManager() {}
SoundManager::~SoundManager() { clear(); }
void SoundManager::clear() { m_buffers.clear(); }
void SoundManager::init() { load("bgm/bgm001.mp3"); }

void SoundManager::load(const std::string& name) {
    fs::path sound_path{std::string(ASSETS_PATH) + "sound/" + name};
    try {

        AudioData data = AudioLoader::load(sound_path);
        auto [_, inserted] = m_buffers.try_emplace(name, data);
        if (!inserted) {
            Logger::error("Key Already exist, check the sound name {}", name);
        }

    } catch (const std::exception& e) {
        Logger::error("Load Sound Error {}", e.what());
    }
}
const AudioBuffer& SoundManager::get_buffer(const std::string& name) const {
    auto it = m_buffers.find(name);
    if (it == m_buffers.end()) {
        std::string err = std::format("Can't Find Buffer {}", name);
        ASSERT_MSG(false, err);
        throw std::runtime_error(err);
    }
    return it->second;
}
} // namespace Cubed