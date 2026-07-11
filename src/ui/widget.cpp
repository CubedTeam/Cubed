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

const std::string& Widget::id() const { return m_id; }

} // namespace Cubed