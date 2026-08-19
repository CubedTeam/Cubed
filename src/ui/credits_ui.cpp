#include "Cubed/ui/credits_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/scene/credits_scene.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/rect.hpp"
#include "Cubed/ui/scroll_view.hpp"

#include <algorithm>
namespace cubed {
CreditsUI::CreditsUI(CreditsScene& scene) : m_scene(scene) {}

void CreditsUI::init() {
    auto& renderer = m_scene.scene_manager().app().renderer();
    update_layout(renderer.window_width(), renderer.window_height());
}

void CreditsUI::update_layout(int, int height) {
    m_root_widget.reset();
    auto image = std::make_unique<Image>(nullptr);
    image
        ->set_image("cubed/textures/ui/background.png",
                    m_scene.scene_manager().app().texture_manager(), false)
        .set_anchor(Anchor::TOP_LEFT)
        .set_fill_parent(true);

    auto& rect = image->add_child<Rect>();
    rect.set_color(Color::BLACK).set_alpha(0.7f).set_fill_parent(true);
    auto& title = rect.add_child<Label>();
    title.set_color(Color::WHITE)
        .set_text(tr("menu.main.credits"))
        .set_anchor(Anchor::TOP_CENTER)
        .set_offset({0, 10});

    auto& return_button = rect.add_child<Button>();
    return_button.set_background_image(
        "cubed/textures/ui/button001.png",
        m_scene.scene_manager().app().texture_manager());

    return_button.set_text(tr("button.return"));
    return_button.set_anchor(Anchor::BOTTOM_CENTER).set_offset({0, -20});
    return_button.set_clicked(
        [this]() { m_scene.scene_manager().request_pop(); });

    {
        auto& scroll = rect.add_child<ScrollView>();
        scroll.set_anchor(Anchor::TOP_CENTER)
            .set_offset({0, 20 + title.height()});

        float scroll_height = height - scroll.get_offset().y -
                              return_button.height() +
                              return_button.get_offset().y;

        scroll.set_height(std::max(0.0f, scroll_height));

        auto layout = std::make_unique<ColumnLayout>(&scroll);
        layout->set_spacing(20).set_anchor(Anchor::TOP_CENTER);
        layout->add_child<Label>().set_text("Cubed");
        auto add_text = [&](std::string_view view, float scale = 0.8f) {
            layout->add_child<Label>().set_text(view).set_scale(scale);
        };
        add_text("A cube game like Minecraft, using C++ and OpenGL.");
        add_text("Author: zhenyan121");
        add_text("Libraries Used", 0.8f);
        add_text("GLAD – MIT / Apache-2.0");
        add_text("SDL3 – zlib");
        add_text("GLM – MIT");
        add_text("FreeType – FTL / GPL-2.0+");
        add_text("toml++ – MIT");
        add_text("Dear ImGui – MIT");
        add_text("Tbb – Apache-2.0");
        add_text("Asio – BSL-1.0");
        add_text("protobuf – BSD-3-Clause");
        add_text("zstd – BSD-3-Clause / GPL-2.0");
        add_text("OpenAL Soft – LGPL-2.1+");
        add_text("Opus – BSD-3-Clause");
        add_text("dr_libs – MIT-0 / Unlicense");
        add_text("RapidJSON  – MIT");
        add_text("HarfBuzz – Old-MIT / GPL-2.0+");
        add_text("utf8cpp – BSL-1.0");
        add_text("stb - public domain / MIT licensed");
        add_text("assimp - BSD-3-Clause");
        add_text("EnTT - MIT licensed");
        add_text("tracy - BSD-3-Clause");
        add_text("RocksDB - Apache License 2.0");
        add_text("libsodium - ISC License");
        add_text("Music", 0.8f);
        add_text("'Find a Peaceful Place' by ROZKOL (Free Music Archive), CC "
                 "BY 4.0.");
        add_text("Special Thanks", 0.8f);
        add_text("TANGERIME");
        add_text("SkyOnPole");
        add_text("free_w_cloud");
        add_text("Last but not least, thanks to you");

        scroll.set_child(std::move(layout));
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

bool CreditsUI::handle_window_resize_event(const WindowResizeEvent& e) {
    update_layout(e.width, e.height);
    return UIManager::handle_window_resize_event(e);
}

} // namespace cubed
