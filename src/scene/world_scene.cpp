#include "Cubed/scene/world_scene.hpp"

#include "Cubed/app.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
namespace Cubed {
WorldScene::WorldScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_dev_panel(*this),
      m_client_world(scene_manager.app().audio(), scene_manager.app().config(),
                     *this),
      m_pasue_menu(*this), m_hud_ui(*this), m_error_ui(*this),
      m_argument(scene_manager.app().argument()) {}

WorldScene::~WorldScene() {
    if (m_client) {
        m_client->stop();
    }
}

void WorldScene::update(float dt) {
    if (m_client->is_connect_error()) {
        set_error(m_client->get_error_string());
        m_client->clear_error();
        m_client_world.set_direct_exit();
    }
    if (m_error_ui.has_error()) {
        m_error_ui.update(dt);
        return;
    }
    m_client_world.update(dt);
    m_camera.update_move_camera();
    m_client_world.get_audio().update_listener(m_camera.get_camera_pos(),
                                               m_camera.get_camera_front(),
                                               glm::vec3(0, 1, 0));

    /*
    const auto& player = m_client_world.get_player();
    if (player_gait != player.get_gait()) {
        player_gait = player.get_gait();
        float fov = m_client_world.get("player.fov", 70.0f);
        if (player_gait == Gait::WALK) {
            m_renderer.update_fov(fov);
        }
        if (player_gait == Gait::RUN) {
            m_renderer.update_fov(fov + 5.0f);
        }
    }*/
    if (m_paused) {
        m_pasue_menu.update(dt);
    } else {
        m_hud_ui.update(dt);
    }
}

void WorldScene::render(Renderer& renderer) {
    if (m_error_ui.has_error()) {
        m_error_ui.render(renderer);
        return;
    }
    renderer.render_world(m_client_world);
    if (m_show_hud) {
        m_hud_ui.render(renderer);
    }
    if (m_paused) {
        m_pasue_menu.render(renderer);
    } else {
        if (m_show_dev_pannel && m_show_hud) {
            renderer.render_dev_panel(m_dev_panel);
        }
    }
}
bool WorldScene::handle_event(const Event& e) {
    if (m_error_ui.has_error()) {
        return m_error_ui.handle_event(e);
    }
    return std::visit(
        Overloaded{[this](const MouseMoveEvent& e) {
                       if (handle_mouse_move_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const MouseButtonEvent& e) {
                       if (handle_mouse_button_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const MouseWheelEvent& e) {
                       if (handle_mouse_wheel_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const KeyEvent& e) {
                       if (handle_key_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const TextInputEvent& e) {
                       if (handle_text_input_event(e)) {
                           return true;
                       }
                       return false;
                   },
                   [this](const WindowResizeEvent& e) {
                       handle_window_resize_event(e);
                       return false;
                   },
                   [](const FrameBufferResizeEvent&) { return false; }

        },
        e);
}
void WorldScene::on_enter() {
    auto& param = m_scene_manager.world_scene_param();

    m_error_ui.init();

    if (param.host_game) {
        if (param.seed) {
            ChunkGenerator::init(*param.seed);
            param.seed = std::nullopt;
        } else {
            ChunkGenerator::init();
        }
        m_server.start_server(param.port);
    }
    m_client = std::make_shared<NetworkClient>(m_client_world);

    m_client->start(param.ip, param.port);
    // init will send packet
    try {

        m_client_world.init(m_argument.player.value_or("Unknown"), m_client);

        Logger::info("World Init Success");
        m_camera.camera_init(&m_client_world.get_player());
        m_scene_manager.app().window().set_camera(&m_camera);
        m_dev_panel.init();
        m_pasue_menu.init();
        m_hud_ui.init();

        m_scene_manager.app().window().set_game_running(true);
    } catch (const std::exception& e) {
        m_error_ui.set_error(e.what());
    }
}
void WorldScene::on_leave() {
    m_client_world.request_exit();

    m_scene_manager.app().window().set_camera(nullptr);
    m_scene_manager.app().window().set_game_running(false);
    auto& param = m_scene_manager.world_scene_param();
    if (param.host_game) {
        m_server.server_world().stop();
    }
    m_scene_manager.app().audio().stop_bgm();
}

void WorldScene::on_re_enter() {
    m_error_ui.on_re_enter();
    m_pasue_menu.on_re_enter();
    m_client_world.reload_config();
    auto width = m_scene_manager.app().renderer().window_width();
    auto height = m_scene_manager.app().renderer().window_height();

    handle_event(
        WindowResizeEvent{static_cast<int>(width), static_cast<int>(height)});
}

bool WorldScene::handle_mouse_move_event(const MouseMoveEvent& e) {
    if (m_paused) {
        if (m_pasue_menu.handle_event(e)) {
            return true;
        }
    } else {
        if (m_hud_ui.handle_event(e)) {
            return true;
        }
        if (m_camera.handle_event(e)) {
            return true;
        }
        // world event needs to be processed last
        if (m_client_world.handle_event(e)) {
            return true;
        }
    }

    return false;
}
bool WorldScene::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (m_paused) {
        if (m_pasue_menu.handle_event(e)) {
            return true;
        }
    } else {
        if (m_hud_ui.handle_event(e)) {
            return true;
        }
        if (m_camera.handle_event(e)) {
            return true;
        }
        // world event needs to be processed last
        if (m_client_world.handle_event(e)) {
            return true;
        }
    }
    return false;
}
bool WorldScene::handle_window_resize_event(const WindowResizeEvent& e) {

    if (m_pasue_menu.handle_event(e)) {
        return true;
    }
    if (m_hud_ui.handle_event(e)) {
        return true;
    }
    if (m_camera.handle_event(e)) {
        return true;
    }
    // world event needs to be processed last
    if (m_client_world.handle_event(e)) {
        return true;
    }

    return false;
}
bool WorldScene::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    if (m_paused) {
        if (m_pasue_menu.handle_event(e)) {
            return true;
        }
    } else {
        if (m_hud_ui.handle_event(e)) {
            return true;
        }
        if (m_camera.handle_event(e)) {
            return true;
        }
        // world event needs to be processed last
        if (m_client_world.handle_event(e)) {
            return true;
        }
    }
    return false;
}
bool WorldScene::handle_key_event(const KeyEvent& e) {

    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
        bool pasued = pause();
        pasued = !pasued;
        set_pause(pasued);
        return true;
    }
    if (e.key == Key::F1 && e.action == KeyAction::PRESS) {
        m_show_hud = !m_show_hud;
        return true;
    }
    if (e.key == Key::F12 && e.action == KeyAction::PRESS) {
        m_show_dev_pannel = !m_show_dev_pannel;
        return true;
    }

    if (e.key == Key::T && e.action == KeyAction::PRESS) {
        if (m_chatting) {
            set_chatting(false);
        } else {
            set_chatting(true);
        }

        return true;
    }

    if (m_paused) {
        if (m_pasue_menu.handle_event(e)) {
            return true;
        }
    } else {
        if (m_hud_ui.handle_event(e)) {
            return true;
        }
        if (m_chatting) {
            return true;
        }
        if (m_camera.handle_event(e)) {
            return true;
        }
        // world event needs to be processed last
        if (m_client_world.handle_event(e)) {
            return true;
        }
    }
    return false;
}

bool WorldScene::handle_text_input_event(const TextInputEvent& e) {
    if (m_paused) {
        return m_pasue_menu.handle_text_input_event(e);
    }
    Logger::info("Hud ui handle text input");
    return m_hud_ui.handle_text_input_event(e);
}

Camera& WorldScene::camera() { return m_camera; }
SceneManager& WorldScene::scene_manager() { return m_scene_manager; }
ClientWorld& WorldScene::client_world() { return m_client_world; }
ServerWorld& WorldScene::server_world() { return m_server.server_world(); }
bool WorldScene::pause() const { return m_paused; }
void WorldScene::set_pause(bool pause) {
    if (m_paused == pause) {
        return;
    }
    m_paused = pause;
    set_mouse(pause);
}

void WorldScene::set_mouse(bool enable) {
    auto& window = m_scene_manager.app().window();
    window.set_game_running(!enable);
    if (enable) {
        m_client_world.reset_key_status();
    }
}

void WorldScene::set_chatting(bool chatting) {
    if (m_paused) {
        return;
    }
    m_chatting = chatting;
    set_mouse(chatting);
    m_hud_ui.set_chatting(chatting);
    Logger::info("World Scene Chatting {}", chatting);
}

// Not thread safe
void WorldScene::handle_chat_message(ChatMessage& message) {
    m_hud_ui.add_chat_message(message);
}

void WorldScene::set_error(std::string_view error) {
    Logger::error("WorldScene Error Set {}", error);
    m_error_ui.set_error(error);
    set_pause(true);
}

} // namespace Cubed