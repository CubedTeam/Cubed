#include "Cubed/ui/world_ui_manager.hpp"

#include "Cubed/app.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/ui/chat_box.hpp"
namespace Cubed {

WorldUIManager::WorldUIManager(WorldScene& scene) : m_scene(scene) {}
WorldUIManager::~WorldUIManager() {}

void WorldUIManager::init() {

    m_root_widget = std::make_unique<Widget>(nullptr);
    m_root_widget->set_fill_parent(true);
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
    auto text_field = std::make_unique<TextField>(&chat_box);
    text_field->set_anchor(Anchor::BOTTOM_LEFT);
    auto rect = std::make_unique<Rect>(text_field.get());
    rect->set_color(Color::GRAY).set_alpha(0.6f).set_fill_parent(true);
    text_field->set_background(std::move(rect))
        .set_app(&m_scene.scene_manager().app())
        .set_fill_width(true)
        .set_visible(false);
    chat_box.set_text_field(std::move(text_field));
    chat_box.set_spacing(15);
    chat_box.set_anchor(Anchor::BOTTOM_LEFT);
    chat_box.set_offset({0, -10});
    chat_box.set_width(200);
    chat_box.set_scale(2.0f);
    chat_box.set_text_scale(0.6f);
    chat_box.set_on_finish([this, &chat_box]() {
        ChatMessage message{m_scene.client_world().get_player().get_name(),
                            std::move(chat_box.get_input_text()), 0};
        chat_box.clear_input();
        m_scene.client_world().send_chat_message(message);
    });
    m_chat_box = &chat_box;
}
void WorldUIManager::render(Renderer& renderer) {
    renderer.begin_render_ui();

    auto& widget = DebugCollector::get().get_widget();
    widget.render(renderer);
    m_root_widget->render(renderer);

    renderer.end_render_ui();
}

void WorldUIManager::set_chatting(bool chantting, bool send) {

    m_chat_box->set_typing(chantting, send);
}

void WorldUIManager::add_chat_message(ChatMessage& message) {

    m_chat_box->add_message(message);
}

bool WorldUIManager::handle_key_event(const KeyEvent& e) {

    return UIManager::handle_key_event(e);
}

} // namespace Cubed