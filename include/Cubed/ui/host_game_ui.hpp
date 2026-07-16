#pragma once

#include "Cubed/ui/label.hpp"
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
    Label* m_error_label;
    void set_error(std::string_view error);
    void clear_error();
};
} // namespace Cubed