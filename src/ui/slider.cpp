#include "Cubed/ui/slider.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <algorithm>
namespace Cubed {

Slider::Slider(Widget* parent) : Widget(parent) {
    init_thumb();
    init_track();

    m_label = std::make_unique<Label>(this);
    m_label->set_anchor(Anchor::CENTER);
    void update_text_scale();
}

Slider& Slider::set_slider(float* value, const float& min, const float& max) {
    m_value = value;
    m_min = min;
    m_max = max;
    return *this;
}

void Slider::init_track() {
    m_track = std::make_unique<Image>(this);

    m_track->set_fill(true);

    m_track->set_anchor(Anchor::TOP_LEFT);
}

void Slider::init_thumb() {
    m_thumb = std::make_unique<Image>(this);

    m_thumb->set_height(height());

    m_thumb->set_width(height() / 2.0f);

    m_thumb->set_anchor(Anchor::TOP_LEFT);

    m_thumb->set_scale(m_scale);
}

float Slider::width() const { return m_width * m_scale; }
float Slider::height() const { return m_height * m_scale; }

Slider& Slider::set_scale(float scale) {
    m_scale = scale;
    m_track->set_width(width()).set_height(height());
    m_thumb->set_height(height());
    m_thumb->set_width(height() / 2.0f);
    void update_text_scale();
    return *this;
}

Slider& Slider::set_width(float width) {
    if (!m_thumb || width < m_thumb->width()) {
        Logger::error("Width is too small set failed!");
        ASSERT(false);
        return *this;
    }
    m_width = width;
    void update_text_scale();
    return *this;
}

Slider& Slider::set_height(float h) {
    m_height = h;
    ASSERT_MSG(m_thumb, "Thumb is nullptr !");
    m_thumb->set_height(height());
    m_thumb->set_width(height() / 2.0f);
    void update_text_scale();
    return *this;
}
Slider& Slider::set_text(const std::string& text) {
    m_text = text;
    return *this;
}
Slider& Slider::set_track_image(const std::string& path,
                                TextureManager& texture_manager) {
    m_track->set_image(path, texture_manager);
    return *this;
}
Slider& Slider::set_thumb_image(const std::string& path,
                                TextureManager& texture_manager) {
    m_thumb->set_image(path, texture_manager);
    return *this;
}

bool Slider::handle_mouse_move_event(const MouseMoveEvent& e) {
    auto p = pos();
    m_xpos = e.xpos;
    if (e.xpos >= p.x && e.xpos <= p.x + width() && e.ypos >= p.y &&
        e.ypos <= p.y + height()) {
        m_inside = true;
    } else {
        m_inside = false;
    }
    if (m_dragging && m_value) {
        update_value(e.xpos);
        return true;
    }
    return Widget::handle_mouse_move_event(e);
}
bool Slider::handle_mouse_button_event(const MouseButtonEvent& e) {
    if (e.action == KeyAction::PRESS && e.key == MouseKey::LEFT_BUTTON) {
        if (m_inside) {
            m_dragging = true;
            update_value(m_xpos);
            return true;
        }
    }

    if (e.action == KeyAction::RELEASE && e.key == MouseKey::LEFT_BUTTON) {
        if (m_dragging) {
            m_dragging = false;
            return true;
        }
    }

    return Widget::handle_mouse_button_event(e);
}

void Slider::update_value(float mouse_x) {
    if (width() <= 0.0f) {
        Logger::info("Slider Width is less then 0");
        return;
    }
    float t = std::clamp((mouse_x - pos().x) / width(), 0.0f, 1.0f);

    *m_value = m_min + t * (m_max - m_min);
}

void Slider::on_update(float dt) {
    if (!m_value || !m_thumb || !m_track) {
        Logger::error("Please Set value and thumb and track");
        ASSERT(false);
        return;
    }
    float range = m_max - m_min;
    if (range <= 0.0f) {
        Logger::error("Range {} is <= 0.0f", range);
        ASSERT(false);
        return;
    }

    float t = (*m_value - m_min) / range;

    t = std::clamp(t, 0.0f, 1.0f);

    float offset = t * (width() - m_thumb->width());

    m_thumb->set_offset({offset, 0});
    if (m_label && m_value) {
        m_label->set_text(std::format("{}: {:.2f}", m_text, *m_value));
    }
    if (m_track) {
        m_track->update(dt);
    }
    if (m_thumb) {
        m_thumb->update(dt);
    }
    if (m_label) {
        m_label->update(dt);
    }
}

void Slider::on_render(Renderer& renderer) {
    if (m_track) {
        m_track->render(renderer);
    }
    if (m_thumb) {
        m_thumb->render(renderer);
    }
    if (m_label) {
        m_label->render(renderer);
    }
}

void Slider::update_text_scale() {
    float text_w = m_label->real_width();
    float text_h = m_label->real_height();

    if (text_w <= 0.0f || text_h <= 0.0f)
        return;

    float available_w = std::max(0.0f, width() - 2.0f * PADDING * m_scale);
    float available_h = std::max(0.0f, height() - 2.0f * PADDING * m_scale);

    float scale_x = available_w / text_w;
    float scale_y = available_h / text_h;

    float scale = std::max(0.0f, std::min(scale_x, scale_y));
    scale = std::clamp(scale, 0.01f, 100.0f);
    m_label->set_scale(scale);
}

} // namespace Cubed