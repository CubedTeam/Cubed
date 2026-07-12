#include "Cubed/scene/world_scene.hpp"

#include "Cubed/app.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
namespace Cubed {
WorldScene::WorldScene(SceneManager& scene_manager)
    : m_scene_manager(scene_manager), m_dev_panel(*this),
      m_client_world(scene_manager.app().audio(), scene_manager.app().config(),
                     *this),
      m_argument(scene_manager.app().argument()) {}

WorldScene::~WorldScene() {
    if (m_client) {
        m_client->stop();
    }
}

void WorldScene::update(float dt) {
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
}

void WorldScene::render(Renderer& renderer) {
    Logger::info("World Scene Render Start !");
    renderer.render_world(m_client_world);
    Logger::info("Client World Render Finish");
    renderer.render_dev_panel(m_dev_panel);
    Logger::info("DevPanel Render Finish!");
}
bool WorldScene::handle_event(const Event& e) {
    if (m_camera.handle_event(e)) {
        return true;
    }
    // world event needs to be processed last
    if (m_client_world.handle_event(e)) {
        return true;
    }
    return false;
}
void WorldScene::on_enter() {

    m_client = std::make_shared<NetworkClient>(m_client_world);

    m_client->start(m_argument.ip, m_argument.port);
    // init will send packet
    m_client_world.init(m_argument.player, m_client);

    Logger::info("World Init Success");
    m_camera.camera_init(&m_client_world.get_player());
    m_scene_manager.app().window().set_camera(&m_camera);
    m_dev_panel.init();
}
void WorldScene::on_leave() {
    m_client_world.request_exit();

    m_scene_manager.app().window().set_camera(nullptr);
}

Camera& WorldScene::camera() { return m_camera; }
SceneManager& WorldScene::scene_manager() { return m_scene_manager; }
ClientWorld& WorldScene::client_world() { return m_client_world; }
} // namespace Cubed