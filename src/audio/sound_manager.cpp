#include "Cubed/audio/sound_manager.hpp"

#include "Cubed/audio/audio_loader.hpp"
#include "Cubed/tools/log.hpp"
#include "Cubed/tools/resource_location.hpp"

#include <filesystem>
namespace fs = std::filesystem;
namespace cubed {
SoundManager::SoundManager() {}
SoundManager::~SoundManager() { clear(); }
void SoundManager::clear() { m_buffers.clear(); }
void SoundManager::init() {
    try {
        load("bgm/bgm001.mp3", false);
        load("bgm/bgm002.ogg", false);
        load("ambient/birds.ogg", false);
    } catch (const std::exception& e) {
    }
}

const AudioBuffer& SoundManager::load(const std::filesystem::path& path,
                                      bool full_path) {
    fs::path root_path = ResourceLocation::get_assets_path_prefix(
        ResourceLocation::DEFAULT_NAMESPACE);

    fs::path sound_path = full_path ? path : root_path / "sounds" / path;
    try {

        AudioData data = AudioLoader::load(sound_path);
        auto [pos, inserted] = m_buffers.try_emplace(path.string(), data);
        if (!inserted) {
            Logger::error("Key Already exist, check the sound name {}",
                          sound_path.string());
        }
        return pos->second;
    } catch (const std::exception& e) {
        Logger::error("Load Sound Error {}", e.what());
        throw;
    }
}
const AudioBuffer& SoundManager::get_buffer(const std::string& name,
                                            bool full_path) {
    auto it = m_buffers.find(name);
    if (it == m_buffers.end()) {
        try {
            return load(name, full_path);
        } catch (const std::exception& e) {
            std::string err = std::format("Can't Find Buffer {}", name);

            throw std::runtime_error(err);
        }
    }
    return it->second;
}
} // namespace cubed
