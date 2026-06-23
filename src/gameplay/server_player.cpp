#include "Cubed/gameplay/server_player.hpp"

namespace Cubed {
ServerPlayer::ServerPlayer(std::string_view name) : m_name(name) {}
const glm::vec3& ServerPlayer::get_pos() const { return m_pos; }
const std::string& ServerPlayer::get_name() const { return m_name; }
void ServerPlayer::update_pos(float x, float y, float z) {
    m_pos = glm::vec3{x, y, z};
}
} // namespace Cubed