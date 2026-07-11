#pragma once
#include "glm/ext/vector_float2.hpp"

#include <memory>
#include <vector>
namespace Cubed {
class Renderer;
class Widget {

public:
    Widget(const std::string& id);
    Widget() = default;
    virtual ~Widget() = default;
    virtual void update(float dt);
    virtual void render(Renderer& renderer);
    virtual const std::string& id() const;
    virtual Widget& set_position(const glm::vec2& pos);
    virtual Widget& set_position(float x, float y);
    virtual Widget& set_scale(float scale);
    virtual const glm::vec2& pos() const;
    virtual float scale() const;
    template <typename T, typename... Args> T& add_child(Args&&... args) {
        auto widget = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *widget;
        m_children.emplace_back(std::move(widget));
        return ref;
    };

protected:
    virtual void on_update(float dt);
    virtual void on_render(Renderer& renderer);
    std::string m_id;
    float m_scale = 1.0f;
    glm::vec2 m_pos{0.0f, 0.0f};

private:
    Widget* m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;
};
} // namespace Cubed