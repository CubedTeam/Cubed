#include "Cubed/ui/error_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/scene/world_scene.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/image.hpp"
namespace Cubed {
ErrorUI::ErrorUI(WorldScene& scene) : m_scene(scene) {}

void ErrorUI::init() {
    auto bi = std::make_unique<Image>(nullptr);
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();

    bi->set_anchor(Anchor::TOP_LEFT);
    bi->set_image("texture/ui/background.png", texture_manager, false);
    bi->set_fill_parent(true);

    auto& rect = bi->add_child<Rect>();
    rect.set_fill_parent(true);
    rect.set_alpha(0.7f);
    rect.set_color(Color::BLACK);

    auto& layout = rect.add_child<ColumnLayout>();
    layout.set_anchor(Anchor::CENTER);
    layout.set_offset({0, 20});
    layout.set_spacing(30.0f);

    {
        auto& label = layout.add_child<Label>();
        label.set_text("No Error");
        label.set_color(Color::RED);
        label.set_scale(0.7f);
        m_error_label = &label;
    }

    {
        auto& button = layout.add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_text(tr("button.return"));
        button.set_clicked([this, &button]() {
            button.set_enable(false);
            m_scene.scene_manager().request_pop();
        });
    }

    m_root_widget = std::move(bi);
}
void ErrorUI::on_re_enter() {}

void ErrorUI::set_error(std::string_view error) {
    m_has_error = true;
    m_error_str = error;
    if (!m_error_str.empty() && m_error_label) {
        m_error_label->set_text(m_error_str);
    }
}
bool ErrorUI::has_error() const { return m_has_error; }
void ErrorUI::clear_error() { m_has_error = false; }

bool ErrorUI::handle_key_event(const KeyEvent& e) {
    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
        m_scene.scene_manager().request_pop();
        return true;
    }
    return UIManager::handle_key_event(e);
}

} // namespace Cubed