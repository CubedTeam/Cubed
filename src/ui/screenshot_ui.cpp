#include "Cubed/ui/screenshot_ui.hpp"

#include "Cubed/app.hpp"
#include "Cubed/localization.hpp"
#include "Cubed/render/renderer.hpp"
#include "Cubed/scene/screenshot_scene.hpp"
#include "Cubed/ui/button.hpp"
#include "Cubed/ui/column_layout.hpp"
#include "Cubed/ui/row_layout.hpp"
#include "Cubed/ui/scroll_view.hpp"

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

    m_image_infos.clear();

    auto& texture_manager = m_scene.scene_manager().app().texture_manager();
    m_root_widget.reset();
    auto bg = std::make_unique<Rect>(nullptr);

    bg->set_color(Color::BLACK).set_fill_parent(true);

    auto& title = bg->add_child<Label>();

    title.set_text(tr("menu.screenshot")).set_anchor(Anchor::TOP_CENTER);
    title.set_offset({0, 5});

    fs::path path = Renderer::SCREENSHOT_PATH;

    constexpr float IMAGE_SCALE = 0.25;
    ScrollView* sv = nullptr;
    if (fs::exists(path)) {
        for (auto& entry : fs::recursive_directory_iterator(path)) {
            if (!fs::is_regular_file(entry)) {
                continue;
            }
            if (!std::ranges::find(IMAGE_EXT, entry.path().extension())) {
                continue;
            }
            ImageInfo info;
            info.texture =
                texture_manager.get_image_texture(entry.path().string(), true);
            info.texture->set_linear();
            info.height = info.texture->height() * IMAGE_SCALE;
            info.width = info.texture->width() * IMAGE_SCALE;
            m_image_infos.emplace_back(std::move(info));
        }
        constexpr int PADDING = 50;
        constexpr int SPACING = 10;

        const int WIDGET_WIDTH = std::min(width, width - 2 * PADDING);

        float cur_width = 0.0f;

        auto& scroll = bg->add_child<ScrollView>();

        sv = &scroll;

        scroll.set_anchor(Anchor::TOP_CENTER)
            .set_offset({0, 10 + title.height()});

        auto root_column = std::make_unique<ColumnLayout>(&scroll);
        root_column->set_child_anchor(ColumnLayoutAnchor::LEFT)
            .set_spacing(10)
            .set_anchor(Anchor::TOP_CENTER);
        RowLayout* row = nullptr;
        auto create_row = [&row, &root_column]() {
            row = &root_column->add_child<RowLayout>();
            row->set_anchor(Anchor::TOP_LEFT);
            row->set_spacing(SPACING);
        };

        for (auto& image : m_image_infos) {
            if (!row) {
                create_row();
            }

            if (cur_width + image.width > WIDGET_WIDTH && cur_width > 0) {
                cur_width = 0.0f;
                create_row();
            }
            cur_width += image.width;
            auto& button = row->add_child<Button>();
            button.set_texture(image.texture, true).set_scale(IMAGE_SCALE);
            button.set_clicked(
                [image, this]() { open_lightbox(image.texture); });
        }
        scroll.set_child(std::move(root_column));
    }
    if (!fs::exists(path) || m_image_infos.empty()) {
        auto& label = bg->add_child<Label>();
        label.set_text(tr("screenshot.no_screenshot"))
            .set_color(Color::WHITE)
            .set_anchor(Anchor::CENTER);
    }
    auto& return_button = bg->add_child<Button>();
    return_button.set_default_image(texture_manager);
    return_button.set_anchor(Anchor::BOTTOM_CENTER);
    return_button.set_offset({0, -30});
    return_button.set_text(tr("button.return"));
    return_button.set_clicked(
        [this]() { m_scene.scene_manager().request_pop(); });
    if (sv) {
        float scroll_height = height - sv->get_offset().y +
                              return_button.get_offset().y -
                              return_button.height();

        sv->set_height(std::max(0.0f, scroll_height));
    }
    auto& rect = bg->add_child<Rect>();
    rect.set_fill_parent(true);
    rect.set_color(Color::BLACK).set_alpha(0.5f);
    m_lightbox = &rect;
    auto& lightbox_image = rect.add_child<Image>();
    m_lightbox_image = &lightbox_image;
    m_lightbox->set_visible(false);
    m_root_widget = std::move(bg);
}

void ScreenshotUI::open_lightbox(const Texture* image) {
    if (image) {
        constexpr float PADDING = 50.0f;
        m_lightbox_image->set_texture(image, true);

        float target_height =
            std::max(50.0f, Widget::get_window_height() - PADDING * 2);
        float target_width =
            std::max(50.0f, Widget::get_window_width() - PADDING * 2);

        float image_width = image->width();
        float image_height = image->height();

        float scale_w = target_width / image_width;
        float scale_h = target_height / image_height;
        float scale = std::min(scale_h, scale_w);
        m_lightbox_image->set_scale(scale);
        m_lightbox_image->set_anchor(Anchor::CENTER);
    }
    m_lightbox->set_visible(true);
}

bool ScreenshotUI::handle_window_resize_event(const WindowResizeEvent& e) {
    update_layout(e.width, e.height);
    return UIManager::handle_window_resize_event(e);
}

bool ScreenshotUI::handle_key_event(const KeyEvent& e) {
    if (UIManager::handle_key_event(e)) {
        return true;
    }
    if (e.key == Key::ESCAPE && e.action == KeyAction::PRESS) {
        if (m_lightbox && m_lightbox->is_visible()) {
            m_lightbox->set_visible(false);
            return true;
        }
        m_scene.scene_manager().request_pop();
        return true;
    }
    return false;
}

bool ScreenshotUI::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (m_lightbox && m_lightbox->is_visible()) {
        if (e.key == MouseKey::LEFT_BUTTON && e.action == KeyAction::PRESS) {
            m_lightbox->set_visible(false);
        }
        return true;
    }
    return UIManager::handle_mouse_button_event(e);
}
bool ScreenshotUI::handle_mouse_move_event(const MouseMoveEvent& e) {
    if (m_lightbox && m_lightbox->is_visible()) {
        return true;
    }
    return UIManager::handle_mouse_move_event(e);
}
bool ScreenshotUI::handle_mouse_wheel_event(const MouseWheelEvent& e) {
    if (m_lightbox && m_lightbox->is_visible()) {
        return true;
    }
    return UIManager::handle_mouse_wheel_event(e);
}

} // namespace Cubed