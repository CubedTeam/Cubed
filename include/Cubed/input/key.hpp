#pragma once

namespace Cubed {
enum class Key {
    // Letter keys
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    // Digit keys (main keyboard area)
    DIGIT_0,
    DIGIT_1,
    DIGIT_2,
    DIGIT_3,
    DIGIT_4,
    DIGIT_5,
    DIGIT_6,
    DIGIT_7,
    DIGIT_8,
    DIGIT_9,

    // Function keys
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    // Control keys
    BACKSPACE,
    TAB,
    ENTER,
    ESCAPE,
    SPACE,
    CAPS_LOCK,
    NUM_LOCK,
    SCROLL_LOCK,

    // Modifier keys
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LEFT_CTRL,
    RIGHT_CTRL,
    LEFT_ALT,
    RIGHT_ALT,
    LEFT_SUPER,
    RIGHT_SUPER, // Windows / Command 键

    // Navigation keys
    INSERT,
    DELETE,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
    LEFT,
    RIGHT,
    UP,
    DOWN,

    // Lock and system keys
    PRINT_SCREEN,
    PAUSE,

    // Main keyboard area symbol keys
    GRAVE_ACCENT,  // `
    MINUS,         // -
    EQUALS,        // =
    LEFT_BRACKET,  // [
    RIGHT_BRACKET, // ]
    BACKSLASH,     // \ /
    SEMICOLON,     // ;
    APOSTROPHE,    // '
    COMMA,         // ,
    PERIOD,        // .
    SLASH,         // /

    // Numpad area
    NUMPAD_0,
    NUMPAD_1,
    NUMPAD_2,
    NUMPAD_3,
    NUMPAD_4,
    NUMPAD_5,
    NUMPAD_6,
    NUMPAD_7,
    NUMPAD_8,
    NUMPAD_9,
    NUMPAD_ADD,      // +
    NUMPAD_SUBTRACT, // -
    NUMPAD_MULTIPLY, // *
    NUMPAD_DIVIDE,   // /
    NUMPAD_DECIMAL,  // .
    NUMPAD_ENTER
};
enum class KeyAction { PRESS, RELEASE, REPEAT };
} // namespace Cubed