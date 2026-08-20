#pragma once

#include "Cubed/ui/color.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
namespace cubed {

struct TextStyle {
    std::string text;
    Color color = Color::WHITE;
};

} // namespace cubed
