#include "Cubed/ui/screenshot_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/screenshot_scene.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/row_layout.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
namespace fs = std::filesystem;
namespace {
constexpr std::array<std::string_view, 4> IMAGE_EXT{".jpg", ".jpeg", ".png",
                                                    ".bmp"};
}
namespace Cubed {
ScreenshotUI::ScreenshotUI(ScreenshotScene& scene) : m_scene(scene) {}
ScreenshotUI::~ScreenshotUI() {}
void ScreenshotUI::init() {

    auto& renderer = m_scene.scene_manager().app().renderer();

    update_layout(renderer.window_width(), renderer.window_height());
}
void ScreenshotUI::update_layout(int width, int height) {
    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    m_root_widget.reset();
    auto bg = std::make_unique<Rect>(nullptr);

    bg->set_color(Color::BLACK).set_fill_parent(true);

    auto& title = bg->add_child<Label>();

    title.set_text(tr("menu.screenshot")).set_anchor(Anchor::TOP_CENTER);
    title.set_offset({0, 5});
    struct ImageInfo {
        const Texture* texture = nullptr;
        int width = 0;
        int height = 0;
    };

    fs::path path = Renderer::SCREENSHOT_PATH;

    std::vector<ImageInfo> image_infos;
    constexpr float IMAGE_SCALE = 0.25;
    if (fs::exists(path)) {
        for (auto& entry : fs::recursive_directory_iterator(path)) {
            if (!fs::is_regular_file(entry)) {
                continue;
            }
            if (!std::ranges::find(IMAGE_EXT, entry.path().extension())) {
                continue;
            }
            ImageInfo info;
            Logger::info("Path: {}", entry.path().string());
            info.texture =
                texture_manager.get_image_texture(entry.path().string(), true);
            info.height = info.texture->height() * IMAGE_SCALE;
            info.width = info.texture->width() * IMAGE_SCALE;
            image_infos.emplace_back(std::move(info));
        }
        constexpr int PADDING = 50;
        constexpr int SPACING = 10;

        const int WIDGET_WIDTH = std::min(width, width - 2 * PADDING);
        // const int WIDGET_HEIGHT = std::max(height, height - 2 * PADDING);

        float cur_width = 0.0f;
        // float cur_height = 0.0f;
        auto& root_column = bg->add_child<ColumnLayout>();
        root_column.set_child_anchor(ColumnLayoutAnchor::LEFT)
            .set_spacing(10)
            .set_anchor(Anchor::TOP_CENTER)
            .set_offset({0, 10 + title.height()});
        RowLayout* row = nullptr;
        auto create_row = [&row, &root_column]() {
            row = &root_column.add_child<RowLayout>();
            row->set_anchor(Anchor::TOP_LEFT);
            row->set_spacing(SPACING);
        };

        for (auto& image : image_infos) {
            if (!row) {
                create_row();
            }

            if (cur_width + image.width > WIDGET_WIDTH && cur_width > 0) {
                cur_width = 0.0f;
                create_row();
            }
            cur_width += image.width;
            auto& im = row->add_child<Image>();
            im.set_texture(image.texture, true).set_scale(IMAGE_SCALE);
        }
    }
    if (!fs::exists(path) || image_infos.empty()) {
        auto& label = bg->add_child<Label>();
        label.set_text(tr("screenshot.no_screenshot"))
            .set_color(Color::WHITE)
            .set_anchor(Anchor::CENTER);
    }

    {
        auto& button = bg->add_child<Button>();
        button.set_default_image(texture_manager);
        button.set_anchor(Anchor::BOTTOM_CENTER);
        button.set_offset({0, -50});
        button.set_text(tr("button.return"));
        button.set_clicked([this]() { m_scene.scene_manager().request_pop(); });
    }

    m_root_widget = std::move(bg);
}
} // namespace Cubed