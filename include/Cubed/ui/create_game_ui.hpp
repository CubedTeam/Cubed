#pragma once

#include "Cubed/ui/label.hpp"
#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class CreateGameScene;
class TextField;
class CreateGameUI : public UIManager {
public:
    CreateGameUI(CreateGameScene& scene);

    void init() override;
    void on_re_enter();

private:
    CreateGameScene& m_scene;
    Label* m_error_label = nullptr;
    TextField* m_world_name_field = nullptr;
    void set_error(std::string_view error);
    void clear_error();
};
} // namespace Cubed
