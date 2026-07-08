#include "Cubed/audio/source_pool.hpp"

namespace Cubed {
SourcePool::SourcePool(size_t size) { m_sources.resize(size); }
SourcePool::~SourcePool() { m_sources.clear(); }
void SourcePool::update() {
    for (auto& source : m_sources) {
        if (source.in_use() && source.state() == AudioState::STOPPED) {
            source.reset();
        }
    }
}

AudioSource* SourcePool::acquire() {
    for (auto& source : m_sources) {
        if (!source.in_use()) {
            source.mark_in_use();
            return &source;
        }
    }
    return nullptr;
}
std::vector<AudioSource>& SourcePool::sources() { return m_sources; }
} // namespace Cubed