#pragma once
#include "Cubed/scene/scene.hpp"

#include <memory>
#include <stack>
#include <vector>
namespace Cubed {
class App;
class SceneManager {
public:
    SceneManager(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;
    SceneManager(App& app);
    ~SceneManager();

    void update(float dt);
    void render(Renderer& renderer);

    bool handle_event(const Event& e);

    void request_change(SceneType type);
    void request_push(SceneType type);
    void request_pop();

    App& app();

private:
    enum class OperationType { PUSH, POP, CHANGE };
    struct SceneOperation {
        OperationType type;
        std::optional<SceneType> scene;
    };

    App& m_app;
    std::vector<std::unique_ptr<Scene>> m_pending_delete_scene;
    std::optional<SceneOperation> m_operation;
    std::stack<std::unique_ptr<Scene>> m_scenes;
    void process_operation();
    void change(SceneType type);
    void push(SceneType type);
    void pop(bool re_enter = true);

    std::unique_ptr<Scene> create_scene(SceneType);
};
} // namespace Cubed