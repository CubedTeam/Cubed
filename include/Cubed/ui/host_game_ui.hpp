#pragma once

#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class HostGameScene;
class HostGameUI : public UIManager {
public:
    HostGameUI(HostGameScene& scene);

    void init() override;
    void on_re_enter();

private:
    HostGameScene& m_scene;
};
} // namespace Cubed