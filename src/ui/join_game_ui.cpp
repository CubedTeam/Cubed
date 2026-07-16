#include "Cubed/ui/join_game_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/scene/join_game_scene.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/text_field.hpp"
namespace Cubed {
JoinGameUI::JoinGameUI(JoinGameScene& scene) : m_scene(scene) {}
void JoinGameUI::init() {
    auto bi = std::make_unique<Image>(nullptr);
    auto& renderer = m_scene.scene_manager().app().renderer();
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();

    bi->set_window_size(renderer.window_width(), renderer.window_height());
    bi->set_anchor(Anchor::TOP_LEFT);
    bi->set_image("texture/ui/background.png", texture_manager);
    bi->set_fill(true);

    auto& rect = bi->add_child<Rect>();
    rect.set_fill(true);
    rect.set_alpha(0.7f);
    rect.set_color(Color::BLACK);

    auto& layout = rect.add_child<ColumnLayout>();
    layout.set_anchor(Anchor::CENTER);
    layout.set_offset({0, 20});
    layout.set_spacing(20.0f);
    auto& param = m_scene.scene_manager().world_scene_param();
    param.host_game = false;
    {
        auto& text_ip = layout.add_child<TextField>();
        text_ip.set_show_text("Server Ip");
        text_ip.set_default_image(texture_manager);
        text_ip.set_app(&m_scene.scene_manager().app());
        text_ip.set_on_finish([this, &text_ip]() {
            auto& ip = text_ip.input_text();
            auto p = ip.find(":");
            if (p == std::string::npos) {
                Logger::error("Missing ':'");
                return;
            }
            auto addr = ip.substr(0, p);
            if (addr.empty()) {
                Logger::error("Empty address");
                return;
            }
            auto port_str = ip.substr(p + 1);
            if (port_str.empty()) {
                Logger::error("Missing port");
                return;
            }
            int port = 25530;
            auto r = std::from_chars(port_str.data(),
                                     port_str.data() + port_str.size(), port);
            if (r.ec != std::errc{} ||
                r.ptr != port_str.data() + port_str.size()) {
                Logger::error("Invalid port: {}", port_str);
                return;
            }
            if (port > 65535 || port < 0) {

                Logger::error("Port {} out of range", port);
                return;
            }
            auto& param = m_scene.scene_manager().world_scene_param();
            param.port = port;
            param.ip = addr;
        });
    }

    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text("Join Game");
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_change(SceneType::WORLD);
        });
    }
    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text("Return");
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_pop();
        });
    }

    m_root_widget = std::move(bi);
}
void JoinGameUI::on_re_enter() {}
} // namespace Cubed