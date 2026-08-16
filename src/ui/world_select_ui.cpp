#include "Cubed/ui/world_select_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/gameplay/server/server_world.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/scene/scene_manager.hpp"
#include "Cubed/scene/world_select_scene.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/default_image.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/rect.hpp"
#include "Cubed/ui/row_layout.hpp"
#include "Cubed/ui/scroll_view.hpp"
#include "Cubed/ui/text_field.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace Cubed {
WorldSelectUI::WorldSelectUI(WorldSelectScene& scene) : m_scene(scene) {}

WorldSelectUI::~WorldSelectUI() {}

void WorldSelectUI::init() {

    auto& renderer = m_scene.scene_manager().app().renderer();
    update_layout(renderer.window_width(), renderer.window_height());
}

void WorldSelectUI::update_layout(int, int height) {
    m_worlds.clear();
    auto bi = std::make_unique<Image>(nullptr);
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();

    bi->set_anchor(Anchor::TOP_LEFT);
    bi->set_image("cubed/textures/ui/background.png", texture_manager, false);
    bi->set_fill_parent(true);

    auto& rect = bi->add_child<Rect>();
    rect.set_fill_parent(true);
    rect.set_alpha(0.7f);
    rect.set_color(Color::BLACK);

    auto& param = m_scene.scene_manager().world_scene_param();
    param.host_game = true;

    auto& title = rect.add_child<Label>();
    title.set_text(tr("hostgame.world_select")).set_anchor(Anchor::TOP_CENTER);
    title.set_offset({0, 5});

    auto& error = rect.add_child<Label>();
    error.set_text("No error")
        .set_color(Color::RED)
        .set_anchor(Anchor::TOP_CENTER)
        .set_visible(false);
    error.set_offset({0, 5 + title.height()});
    m_error = &error;
    float up_height = 10 + title.height() + error.height();

    auto& scroll = rect.add_child<ScrollView>();
    scroll.set_anchor(Anchor::TOP_CENTER);

    std::vector<fs::path> worlds;

    fs::path save = ServerWorld::SAVE_ROOT;

    for (auto& world : fs::directory_iterator(
             save, fs::directory_options::skip_permission_denied)) {
        if (!world.is_directory()) {
            continue;
        }

        worlds.emplace_back(world.path());
    }

    {
        auto layout = std::make_unique<ColumnLayout>(&scroll);
        layout->set_anchor(Anchor::TOP_CENTER);
        layout->set_spacing(20.0f);

        for (const auto& world : worlds) {
            auto& button = layout->add_child<Button>();

            button.set_text(world.filename());
            button.set_border_size(2);
            button.set_border_visale(false);
            button.set_mouse_highlight(false);
            button.set_clicked([this, world, &button]() {
                m_select_world = world.filename();
                button.set_border_visale(true);

                for (auto w : m_worlds) {
                    if (&button == w) {
                        continue;
                    }
                    if (w) {
                        w->set_border_visale(false);
                    }
                }
            });
            m_worlds.emplace_back(&button);
        }

        scroll.set_child(std::move(layout));
    }
    auto& bottom_layout = rect.add_child<ColumnLayout>();

    bottom_layout.set_spacing(20);
    bottom_layout.set_child_anchor(ColumnLayoutAnchor::CENTER);
    bottom_layout.set_anchor(Anchor::BOTTOM_CENTER);
    {

        {
            {
                auto& text_port = bottom_layout.add_child<TextField>();
                std::unique_ptr<Image> back =
                    std::make_unique<Image>(&text_port);
                back->set_image(DEFAULT_TEXT_FIELD_IMAGE, texture_manager,
                                false)
                    .set_fill_parent(true);
                text_port.set_background(std::move(back));
                text_port.set_show_text(tr("hostgame.port"));
                text_port.set_app(&m_scene.scene_manager().app());
                text_port.set_on_finish([this, &text_port]() {
                    int port = 25530;
                    auto& text = text_port.input_text();
                    auto r = std::from_chars(text.data(),
                                             text.data() + text.size(), port);
                    if (r.ec != std::errc{} ||
                        r.ptr != text.data() + text.size()) {
                        std::string error =
                            std::format("Invalid port: {}", text);
                        Logger::error("{}", error);
                        set_error(error);
                        return;
                    }
                    if (port > 65535 || port < 0) {
                        std::string error =
                            std::format("Port {} out of range", port);
                        Logger::error("{}", error);
                        set_error(error);
                        return;
                    }
                    clear_error();
                    m_scene.scene_manager().world_scene_param().port = port;
                });
            }
        }
        auto& world_row = bottom_layout.add_child<RowLayout>();
        world_row.set_spacing(10);
        auto& enter = world_row.add_child<Button>();
        enter.set_default_image(texture_manager);
        enter.set_text(tr("hostgame.enter_world"));
        enter.set_clicked([this, &param]() {
            if (m_select_world.empty()) {
                set_error(tr("hsotgame.you_must_select_a_world"));
                return;
            }
            param.world_name = m_select_world;
            m_scene.scene_manager().request_change(SceneType::WORLD);
        });
        auto& create = world_row.add_child<Button>();
        create.set_default_image(texture_manager);
        create.set_text(tr("hostgame.create_world"));
        create.set_clicked([this]() {
            m_scene.scene_manager().request_change(SceneType::HOST_GAME);
        });
        auto& return_button = bottom_layout.add_child<Button>();
        return_button.set_default_image(texture_manager);
        return_button.set_text(tr("button.return"));
        return_button.set_clicked(
            [this]() { m_scene.scene_manager().request_pop(); });
    }
    bottom_layout.layout();
    float botton_height = bottom_layout.height();

    scroll.set_offset({0, up_height + 5});
    scroll.set_height(std::max(0.0f, height - botton_height));

    m_root_widget = std::move(bi);
}

void WorldSelectUI::set_error(std::string_view error) {
    if (m_error) {
        m_error->set_text(error);
        m_error->set_visible(true);
    }
}
void WorldSelectUI::clear_error() {
    if (m_error) {
        m_error->set_text("No error");
        m_error->set_visible(false);
    }
}

bool WorldSelectUI::handle_window_resize_event(const WindowResizeEvent& e) {
    update_layout(e.width, e.height);
    return UIManager::handle_window_resize_event(e);
}

} // namespace Cubed