#pragma once

#include "Cubed/input/key.hpp"
#include "Cubed/input/mouse.hpp"

#include <string>
#include <variant>

namespace Cubed {

struct MouseMoveEvent {
    float xpos;
    float ypos;
    float xrel;
    float yrel;
    MouseMoveEvent(float x, float y, float dx, float dy)
        : xpos(x), ypos(y), xrel(dx), yrel(dy) {}
};

struct MouseButtonEvent {
    MouseKey key;
    KeyAction action;

    MouseButtonEvent(MouseKey k, KeyAction a) : key(k), action(a) {}
};

struct MouseWheelEvent {
    float offset;

    MouseWheelEvent(float o) : offset(o) {}
};

struct KeyEvent {
    Key key;
    KeyAction action;

    KeyEvent(Key k, KeyAction a) : key(k), action(a) {}
};

struct TextInputEvent {
    std::string text;

    TextInputEvent(std::string t) : text(std::move(t)) {}
};

struct WindowResizeEvent {
    int width;
    int height;
};
struct FrameBufferResizeEvent {
    int width;
    int height;
};
using Event =
    std::variant<MouseMoveEvent, MouseButtonEvent, MouseWheelEvent, KeyEvent,
                 TextInputEvent, WindowResizeEvent, FrameBufferResizeEvent>;

template <class... T> struct Overloaded : T... {
    using T::operator()...;
};
template <class... T> Overloaded(T...) -> Overloaded<T...>;

} // namespace Cubed