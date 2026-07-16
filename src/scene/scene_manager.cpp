#include "Cubed/scene/scene_manager.hpp"

#include "Cubed/scene/credits_scene.hpp"
#include "Cubed/scene/host_game_scene.hpp"
#include "Cubed/scene/join_game_scene.hpp"
#include "Cubed/scene/main_menu_scene.hpp"
#include "Cubed/scene/settings_scene.hpp"
#include "Cubed/scene/world_scene.hpp"

namespace Cubed {
SceneManager::SceneManager(App& app) : m_app(app) {}
SceneManager::~SceneManager() {}

void SceneManager::update(float dt) {
    m_pending_delete_scene.clear();
    process_operation();
    if (!m_scenes.empty()) {
        m_scenes.top()->update(dt);
    }
}
void SceneManager::render(Renderer& renderer) {
    if (m_scenes.empty()) {
        return;
    }
    m_scenes.top()->render(renderer);
}

bool SceneManager::handle_event(const Event& e) {
    if (m_scenes.empty()) {
        return false;
    }
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
        pop(false);
    }

    push(type);
}
void SceneManager::push(SceneType type) {
    auto scene = create_scene(type);
    scene->on_enter();
    m_scenes.push(std::move(scene));
}
void SceneManager::pop(bool re_enter) {
    if (m_scenes.empty()) {
        return;
    }
    auto scene = std::move(m_scenes.top());
    m_scenes.pop();
    scene->on_leave();
    m_pending_delete_scene.push_back(std::move(scene));
    if (!m_scenes.empty() && re_enter) {
        m_scenes.top()->on_re_enter();
    }
}

std::unique_ptr<Scene> SceneManager::create_scene(SceneType type) {
    switch (type) {
    case SceneType::WORLD:
        return std::make_unique<WorldScene>(*this);
    case SceneType::MAIN_MENU:
        return std::make_unique<MainMenuScene>(*this);
    case SceneType::CREDITS:
        return std::make_unique<CreditsScene>(*this);
    case SceneType::SETTINGS:
        return std::make_unique<SettingsScene>(*this);
    case SceneType::HOST_GAME:
        return std::make_unique<HostGameScene>(*this);
    case SceneType::JOIN_GAME:
        return std::make_unique<JoinGameScene>(*this);
    }

    std::string err = std::format("Unknown Scene");
    ASSERT_MSG(false, err);
    throw std::runtime_error(err);
}

App& SceneManager::app() { return m_app; }
WorldSceneParam& SceneManager::world_scene_param() { return m_world_param; }
} // namespace Cubed