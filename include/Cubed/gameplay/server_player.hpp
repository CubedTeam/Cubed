#pragma once
#include <glm/glm.hpp>
#include <string>
#include <string_view>
namespace Cubed {
class ServerPlayer {
public:
    explicit ServerPlayer(std::string_view);
    const glm::vec3& get_pos() const;
    const std::string& get_name() const;
    void update_pos(float x, float y, float z);

private:
    std::string m_name;
    glm::vec3 m_pos{0.0f};
};
} // namespace Cubed
