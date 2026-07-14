#include "Cubed/ui/slider.hpp"

#include "Cubed/tools/cubed_assert.hpp"
#include "Cubed/tools/log.hpp"

#include <algorithm>
namespace Cubed {

Slider::Slider(Widget* parent) : Widget(parent) {
    init_thumb();
    init_track();
}

Slider& Slider::set_slider(float* value, const float& min, const float& max) {
    m_value = value;
    m_min = min;
    m_max = max;
    return *this;
}

Image& Slider::get_track() { return *m_track; }
Image& Slider::get_thumb() { return *m_thumb; }

void Slider::init_track() {
    m_track = std::make_unique<Image>(this);

    m_track->set_fill(true);

    m_track->set_anchor(Anchor::TOP_LEFT);
}

void Slider::init_thumb() {
    m_thumb = std::make_unique<Image>(this);

    m_thumb->set_height(height());

    m_thumb->set_width(THUMB_WIDTH);

    m_thumb->set_anchor(Anchor::TOP_LEFT);

    m_thumb->set_scale(m_scale);
}

float Slider::width() const { return m_width * m_scale; }
float Slider::height() const { return m_height * m_scale; }

Slider& Slider::set_scale(float scale) {
    m_scale = scale;
    m_track->set_width(width()).set_height(height());
    m_thumb->set_height(height());
    return *this;
}

Slider& Slider::set_width(float width) {
    if (!m_thumb || width < m_thumb->width()) {
        Logger::error("Width is too small set failed!");
        ASSERT(false);
        return *this;
    }
    m_width = width;
    return *this;
}

Slider& Slider::set_height(float h) {
    m_height = h;
    ASSERT_MSG(m_thumb, "Thumb is nullptr !");
    m_thumb->set_height(height());
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

void Slider::on_update(float) {
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
}

void Slider::on_render(Renderer& renderer) {
    if (m_track) {
        m_track->render(renderer);
    }
    if (m_thumb) {
        m_thumb->render(renderer);
    }
}

} // namespace Cubed