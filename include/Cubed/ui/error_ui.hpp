#pragma once

#include "Cubed/ui/label.hpp"
#include "Cubed/ui/ui_manager.hpp"
namespace Cubed {
class WorldScene;
class ErrorUI : public UIManager {
public:
    ErrorUI(WorldScene& scene);

    void init() override;
    void on_re_enter();
    void set_error(std::string_view error);
    bool has_error() const;
    void clear_error();

private:
    bool handle_key_event(const KeyEvent& e) override;
    WorldScene& m_scene;
    bool m_has_error = false;
    std::string m_error_str;
    Label* m_error_label = nullptr;
};
} // namespace Cubed