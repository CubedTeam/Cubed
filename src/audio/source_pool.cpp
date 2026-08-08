#include "Cubed/audio/source_pool.hpp"

#include "Cubed/tools/log.hpp"

namespace Cubed {
SourcePool::SourcePool(size_t size) {
    m_sources.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        m_sources.emplace_back(std::make_unique<AudioSource>());
    }
}
SourcePool::~SourcePool() { m_sources.clear(); }
void SourcePool::update() {
    for (auto& source_ptr : m_sources) {
        if (!source_ptr->in_use()) {
            continue;
        }

        auto state = source_ptr->state();

        if (state == AudioState::STOPPED || state == AudioState::INITIAL) {
            source_ptr->reset();
        }
    }
}

AudioSource* SourcePool::acquire() {
    for (auto& source_ptr : m_sources) {
        if (!source_ptr->in_use()) {
            source_ptr->mark_in_use();
            return source_ptr.get();
        }
    }
    return nullptr;
}
std::vector<std::unique_ptr<AudioSource>>& SourcePool::sources() {
    return m_sources;
}
} // namespace Cubed