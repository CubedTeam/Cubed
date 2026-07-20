#include "Cubed/ui/credits_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/scene/credits_scene.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/rect.hpp"

namespace Cubed {
CreditsUI::CreditsUI(CreditsScene& scene) : m_scene(scene) {}

void CreditsUI::init() {
    auto& renderer = m_scene.scene_manager().app().renderer();

    auto image = std::make_unique<Image>(nullptr);
    image
        ->set_image("texture/ui/background.png",
                    m_scene.scene_manager().app().texture_manager())
        .set_anchor(Anchor::TOP_LEFT)
        .set_window_size(renderer.window_width(), renderer.window_height())
        .set_fill_parent(true);

    auto& rect = image->add_child<Rect>();
    rect.set_color(Color::BLACK).set_alpha(0.7f).set_fill_parent(true);

    {
        auto& button = rect.add_child<Button>();
        button.set_background_image(
            "texture/ui/button001.png",
            m_scene.scene_manager().app().texture_manager());

        button.set_text(tr("button.return"));
        button.set_anchor(Anchor::BOTTOM_CENTER).set_offset({0, -20});
        button.set_clicked([this]() { m_scene.scene_manager().request_pop(); });
    }
    {
        auto& layout = rect.add_child<ColumnLayout>();
        layout.set_spacing(20)
            .set_anchor(Anchor::TOP_CENTER)
            .set_offset({0, 20});
        layout.add_child<Label>().set_text("Cubed");
        auto add_text = [&](std::string_view view, float scale = 0.5f) {
            layout.add_child<Label>().set_text(view).set_scale(scale);
        };
        add_text("A cube game like Minecraft, using C++ and OpenGL.");
        add_text("Author: zhenyan121");
        add_text("Libraries Used", 0.8f);
        add_text("glad");
        add_text("SDL3");
        add_text("SOIL2");
        add_text("GLM");
        add_text("FreeType");
        add_text("toml++");
        add_text("Dear ImGui");
        add_text("Tbb");
        add_text("Asio");
        add_text("protobuf");
        add_text("zstd");
        add_text("OpenAl Soft");
        add_text("Opus");
        add_text("dr_libs");
        add_text("nlohmann/json");
        add_text("HarfBuzz");
        add_text("utf8cpp");
        add_text("Music", 0.8f);
        add_text("'Find a Peaceful Place' by ROZKOL (Free Music Archive), CC "
                 "BY 4.0.");
        add_text("Special Thanks", 0.8f);
        add_text("TANGERIME");
        add_text("SkyOnPole");
        add_text("free_w_cloud");
        add_text("Last but not least, thanks to you");
    }

    m_root_widget = std::move(image);
}

bool CreditsUI::handle_key_event(const KeyEvent& e) {
    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
        m_scene.scene_manager().request_pop();
        return true;
    }
    return UIManager::handle_key_event(e);
}

} // namespace Cubed