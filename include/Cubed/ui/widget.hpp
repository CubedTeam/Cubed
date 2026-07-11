#pragma once
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

private:
    Widget* m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;
};
} // namespace Cubed