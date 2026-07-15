#pragma once
#include "Cubed/scene/scene.hpp"

#include <memory>
#include <stack>
#include <vector>
namespace Cubed {
class App;

struct WorldSceneParam {
    bool host_game = true;
    std::optional<unsigned> seed = std::nullopt;
    std::string ip{"127.0.0.1"};
    int port = 25530;
};

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
    WorldSceneParam& world_scene_param();

private:
    enum class OperationType { PUSH, POP, CHANGE };
    struct SceneOperation {
        OperationType type;
        std::optional<SceneType> scene;
    };

    App& m_app;
    WorldSceneParam m_world_param;
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