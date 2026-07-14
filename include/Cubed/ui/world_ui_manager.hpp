#pragma once

#include "Cubed/ui/ui_manager.hpp"
#include "Cubed/ui/widget.hpp"

namespace Cubed {
class WorldScene;
class WorldUIManager : public UIManager {
public:
    WorldUIManager(const WorldUIManager&) = delete;
    WorldUIManager(WorldUIManager&&) = delete;
    WorldUIManager& operator=(const WorldUIManager&) = delete;
    WorldUIManager& operator=(WorldUIManager&&) = delete;

    WorldUIManager(WorldScene& scene);
    ~WorldUIManager();

    void init() override;
    void render(Renderer& renderer) override;

private:
    WorldScene& m_scene;
};
} // namespace Cubed