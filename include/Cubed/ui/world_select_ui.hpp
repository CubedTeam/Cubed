#pragma once

#include "Cubed/ui/button.hpp"
#include "Cubed/ui/label.hpp"
#include "Cubed/ui/ui_manager.hpp"
namespace cubed {
class WorldSelectScene;
class WorldSelectUI : public UIManager {
public:
    WorldSelectUI(WorldSelectScene& scene);
    ~WorldSelectUI();
    void init() override;
    bool handle_window_resize_event(const WindowResizeEvent& e) override;

private:
    WorldSelectScene& m_scene;
    std::string m_select_world;
    Label* m_error;
    std::vector<Button*> m_worlds;
    void update_layout(int width, int height);
    void set_error(std::string_view error);
    void clear_error();
};
} // namespace cubed
