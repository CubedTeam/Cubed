#pragma once

namespace Cubed {
class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;
    virtual void update(float dt) = 0;
    virtual void render() = 0;
};
} // namespace Cubed