#pragma once
#include "Cubed/input/event.hpp"
namespace Cubed {
class Renderer;

enum class SceneType { MAIN_MENU, WORLD };

class Scene {
public:
    Scene() = default;

    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

    virtual ~Scene() = default;
    virtual void update(float dt) = 0;
    virtual void render(Renderer& renderer) = 0;
    virtual bool handle_event(const Event& e) = 0;
    virtual void on_enter() {};
    virtual void on_leave() {};
};
} // namespace Cubed