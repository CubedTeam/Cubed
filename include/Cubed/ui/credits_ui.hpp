#pragma once

#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {

class CreditsScene;

class CreditsUI : public UIManager {
public:
    CreditsUI(CreditsScene& m_scene);
    void init() override;

private:
    CreditsScene& m_scene;
};
} // namespace Cubed