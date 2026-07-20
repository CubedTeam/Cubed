#pragma once
#include "Cubed/audio/audio_source.hpp"

namespace Cubed {
class SourcePool {
private:
    std::vector<std::unique_ptr<AudioSource>> m_sources;

public:
    explicit SourcePool(size_t size);
    ~SourcePool();
    SourcePool(const SourcePool&) = delete;
    SourcePool(SourcePool&&) = delete;
    SourcePool& operator=(const SourcePool&) = delete;
    SourcePool& operator=(SourcePool&&) = delete;

    void update();
    [[nodiscard]]
    AudioSource* acquire();

    std::vector<std::unique_ptr<AudioSource>>& sources();
};

} // namespace Cubed