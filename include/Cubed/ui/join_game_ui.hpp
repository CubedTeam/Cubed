#pragma once

#include "Cubed/ui/label.hpp"
#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class JoinGameScene;
class JoinGameUI : public UIManager {
public:
    JoinGameUI(const JoinGameUI&) = delete;
    JoinGameUI(JoinGameUI&&) = delete;
    JoinGameUI& operator=(const JoinGameUI&) = delete;
    JoinGameUI& operator=(JoinGameUI&&) = delete;

    JoinGameUI(JoinGameScene& m_scene);

    void init() override;
    void on_re_enter();

private:
    JoinGameScene& m_scene;
    Label* m_error_label;
    void set_error(std::string_view error);
    void clear_error();
};
} // namespace Cubed