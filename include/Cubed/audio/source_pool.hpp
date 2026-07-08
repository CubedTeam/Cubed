#pragma once
#include "Cubed/audio/audio_source.hpp"

namespace Cubed {
class SourcePool {
private:
    std::vector<AudioSource> m_sources;

public:
    explicit SourcePool(size_t size);
    ~SourcePool();
    SourcePool(const SourcePool&) = delete;
    SourcePool(SourcePool&&) = delete;
    SourcePool& operator=(const SourcePool&) = delete;
    SourcePool& operator=(SourcePool&&) = delete;

    void update();
    AudioSource* acquire();

    std::vector<AudioSource>& sources();
};

} // namespace Cubed