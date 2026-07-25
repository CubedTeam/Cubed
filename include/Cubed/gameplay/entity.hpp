#pragma once
#include <glm/glm.hpp>
#include <string>
namespace Cubed {
struct Transform {
    glm::vec3 pos;
};

struct Model {
    std::string name;
};

struct Health {
    float hp = 0;
    float max_hp = 20;
};

} // namespace Cubed