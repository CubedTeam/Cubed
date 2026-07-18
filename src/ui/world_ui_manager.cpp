#include "Cubed/ui/world_ui_manager.hpp"

#include "Cubed/app.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/ui/chat_box.hpp"
namespace Cubed {

WorldUIManager::WorldUIManager(WorldScene& scene) : m_scene(scene) {}
WorldUIManager::~WorldUIManager() {}

void WorldUIManager::init() {

    m_root_widget = std::make_unique<Widget>(nullptr);
    auto& renderer = m_scene.scene_manager().app().renderer();

    m_root_widget->set_window_size(renderer.window_width(),
                                   renderer.window_height());

    auto& crosshair = m_root_widget->add_child<Image>();

    crosshair.set_image("texture/ui/0.png",
                        m_scene.scene_manager().app().texture_manager());
    crosshair.set_window_size(renderer.window_width(),
                              renderer.window_height());
    crosshair.set_anchor(Anchor::CENTER);
    crosshair.set_scale(3.0f);
    m_widgets.try_emplace("crosshair", &crosshair);

    auto& chat_box = m_root_widget->add_child<ChatBox>();
    chat_box.set_spacing(15);
    chat_box.set_anchor(Anchor::BOTTOM_LEFT);
    chat_box.set_offset({0, -10});
    chat_box.set_width(200);
    chat_box.set_scale(2.0f);
    chat_box.set_text_scale(0.6f);
    chat_box.set_d_image(m_scene.scene_manager().app().texture_manager());
    chat_box.set_app(&m_scene.scene_manager().app());
    chat_box.set_on_finish([]() {});
    m_chat_box = &chat_box;
}
void WorldUIManager::render(Renderer& renderer) {
    renderer.begin_render_ui();

    auto& widget = DebugCollector::get().get_widget();
    widget.render(renderer);
    m_root_widget->render(renderer);

    renderer.end_render_ui();
}

void WorldUIManager::set_chatting(bool chantting) {
    Logger::info("WorldUIManager Chatting {}", chantting);
    m_chat_box->set_typing(chantting);
}

bool WorldUIManager::handle_key_event(const KeyEvent& e) {

    return UIManager::handle_key_event(e);
}

} // namespace Cubed