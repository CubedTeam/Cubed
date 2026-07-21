#include "Cubed/ui/world_ui_manager.hpp"

#include "Cubed/app.hpp"
#include "Cubed/debug_collector.hpp"
#include "Cubed/gameplay/client_world.hpp"
#include "Cubed/localization.hpp"
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
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    auto& crosshair = m_root_widget->add_child<Image>();

    crosshair.set_image("texture/ui/0.png", texture_manager, true);

    crosshair.set_anchor(Anchor::CENTER);
    crosshair.set_scale(3.0f);
    m_widgets.try_emplace("crosshair", &crosshair);
    {
        auto& hotbar = m_root_widget->add_child<RowLayout>();
        hotbar.set_anchor(Anchor::BOTTOM_CENTER);
        m_hotbar = &hotbar;
        for (size_t i = 0; i < ClientPlayer::HOTBAR_SUM; ++i) {
            auto& bg = hotbar.add_child<Image>();
            bg.set_image("texture/ui/slot.png", texture_manager, true);
            bg.set_scale(5.0f);
            auto& item = bg.add_child<Image>();
            item.set_fill_parent(true).set_anchor(Anchor::CENTER);
            m_hotbar_items.emplace_back(&item);
            m_hotbar_slot.emplace_back(&bg);
        }
        update_hotbar();
    }
    auto& chat_box = m_root_widget->add_child<ChatBox>();
    auto text_field = std::make_unique<TextField>(&chat_box);
    text_field->set_anchor(Anchor::BOTTOM_LEFT);
    auto rect = std::make_unique<Rect>(text_field.get());
    rect->set_color(Color::GRAY).set_alpha(0.6f).set_fill_parent(true);
    text_field->set_background(std::move(rect))
        .set_app(&m_scene.scene_manager().app())
        .set_fill_width(true)
        .set_visible(false)
        .set_height(15.0f);
    chat_box.set_text_field(std::move(text_field));
    chat_box.set_spacing(0);
    chat_box.set_anchor(Anchor::BOTTOM_LEFT);
    chat_box.set_offset({0, -10});
    // chat_box.set_width(500);
    chat_box.set_fill_width(true);
    // chat_box.set_scale(2.0f);
    chat_box.set_text_scale(0.6f);
    chat_box.set_on_finish([this, &chat_box]() {
        ChatMessage message{m_scene.client_world().get_player().get_name(),
                            std::move(chat_box.get_input_text()), Color::WHITE,
                            false, 0};
        chat_box.clear_input();
        m_scene.client_world().send_chat_message(message);
    });
    m_chat_box = &chat_box;
    auto& microphone = m_root_widget->add_child<Image>();
    microphone.set_image("texture/ui/microphone.png", texture_manager, true)
        .set_scale(5.0f)
        .set_anchor(Anchor::BOTTOM_RIGHT)
        .set_visible(false);
    m_mircophone = &microphone;

    auto& disbale_voice = m_root_widget->add_child<Label>();
    disbale_voice.set_scale(0.6f);
    disbale_voice.set_text(tr("error.disable_voice"))
        .set_color(Color::RED)
        .set_offset({0, -5});
    disbale_voice.set_anchor(Anchor::BOTTOM_RIGHT).set_visible(false);
    m_disbable_voice = &disbale_voice;
}
void WorldUIManager::render(Renderer& renderer) {
    renderer.begin_render_ui();

    auto& widget = DebugCollector::get().get_widget();
    widget.render(renderer);
    m_root_widget->render(renderer);

    renderer.end_render_ui();
}

void WorldUIManager::update(float dt) {
    UIManager::update(dt);
    if (m_scene.is_recording()) {
        if (m_scene.client_world().enable_voice_chat()) {
            m_mircophone->set_visible(true);
        } else {
            m_disbable_voice->set_visible(true);
        }

    } else {
        m_mircophone->set_visible(false);
        m_disbable_voice->set_visible(false);
    }

    update_hotbar();
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

void WorldUIManager::update_hotbar() {
    if (!m_hotbar) {
        return;
    }
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    auto& item_texture = texture_manager.get_item_textures();
    auto& player = m_scene.client_world().get_player();
    auto hotbar = player.get_hotbar();
    size_t selected = player.selected_hotbar();
    for (size_t i = 0; i < ClientPlayer::HOTBAR_SUM; ++i) {
        auto type = hotbar[i].type;
        if (selected == i) {
            m_hotbar_slot[i]->set_border_visale(true);
        } else {
            m_hotbar_slot[i]->set_border_visale(false);
        }
        if (type == 0) {
            m_hotbar_items[i]->set_texture(nullptr, false);
        } else {
            m_hotbar_items[i]->set_texture(item_texture[type].get(), false);
        }
    }
}

} // namespace Cubed