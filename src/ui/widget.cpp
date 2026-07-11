#include "Cubed/ui/widget.hpp"

namespace Cubed {
Widget::Widget(const std::string& id) : m_id(id) {}
void Widget::update(float dt) {
    on_update(dt);

    for (auto& child : m_children) {
        child->update(dt);
    }
}

void Widget::render(Renderer& renderer) {
    on_render(renderer);

    for (auto& child : m_children) {
        child->render(renderer);
    }
}

void Widget::on_update(float dt) {}
void Widget::on_render(Renderer& renderer) {}

Widget& Widget::set_position(const glm::vec2& pos) {
    return set_position(pos.x, pos.y);
}

Widget& Widget::set_position(float x, float y) {
    m_pos.x = x;
    m_pos.y = y;
    return *this;
}

Widget& Widget::set_scale(float scale) {
    m_scale = scale;
    return *this;
}
const glm::vec2& Widget::pos() const { return m_pos; }
float Widget::scale() const { return m_scale; }
const std::string& Widget::id() const { return m_id; }

} // namespace Cubed