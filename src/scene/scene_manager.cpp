#include "Cubed/scene/scene_manager.hpp"

#include "Cubed/scene/world_scene.hpp"

namespace Cubed {
SceneManager::SceneManager(App& app) : m_app(app) {}
SceneManager::~SceneManager() {}

void SceneManager::update(float dt) {
    m_pending_delete_scene.clear();

    if (!m_scenes.empty()) {
        m_scenes.top()->update(dt);
    }
    process_operation();
}
void SceneManager::render(Renderer& renderer) {
    if (m_scenes.empty()) {
        return;
    }
    m_scenes.top()->render(renderer);
}

bool SceneManager::handle_event(const Event& e) {
    return m_scenes.top()->handle_event(e);
}

void SceneManager::request_change(SceneType type) {
    ASSERT(!m_operation.has_value());
    m_operation = {OperationType::CHANGE, type};
}
void SceneManager::request_push(SceneType type) {
    ASSERT(!m_operation.has_value());
    m_operation = {OperationType::PUSH, type};
}
void SceneManager::request_pop() {
    ASSERT(!m_operation.has_value());
    m_operation = {OperationType::POP, std::nullopt};
}

void SceneManager::process_operation() {
    while (m_operation) {
        auto op = std::move(*m_operation);
        m_operation.reset();

        switch (op.type) {
        case OperationType::PUSH:
            push(*op.scene);
            break;
        case OperationType::POP:
            pop();
            break;
        case OperationType::CHANGE:
            change(*op.scene);
            break;
        }
    }
}

void SceneManager::change(SceneType type) {
    if (!m_scenes.empty()) {
        pop();
    }

    push(type);
}
void SceneManager::push(SceneType type) {
    auto scene = create_scene(type);
    scene->on_enter();
    m_scenes.push(std::move(scene));
}
void SceneManager::pop() {
    if (m_scenes.empty()) {
        return;
    }
    auto scene = std::move(m_scenes.top());
    m_scenes.pop();
    scene->on_leave();
    m_pending_delete_scene.push_back(std::move(scene));
}

std::unique_ptr<Scene> SceneManager::create_scene(SceneType type) {
    switch (type) {
    case SceneType::WORLD:
        return std::make_unique<WorldScene>(*this);
    case SceneType::MAIN_MENU:
        return nullptr;
    }

    std::string err = std::format("Unknown Scene");
    ASSERT_MSG(false, err);
    throw std::runtime_error(err);
}

App& SceneManager::app() { return m_app; }
} // namespace Cubed