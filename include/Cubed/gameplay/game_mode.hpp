#pragma once

#include <stdexcept>
#include <string>

namespace cubed {

enum class GameMode { CREATIVE = 0, SURVIVAL = 1, SPECTATOR = 2 };

constexpr std::string to_str(GameMode mode) {
    using enum GameMode;
    switch (mode) {
    case CREATIVE:
        return {"Creative"};
    case SPECTATOR:
        return {"Spective"};
    case SURVIVAL:
        return {"Survival"};
    }

    throw std::invalid_argument{"GameMode is invaild"};
}

constexpr GameMode get_game_mode(int id) {
    switch (id) {
    case 0:
        return GameMode::CREATIVE;
    case 1:
        return GameMode::SURVIVAL;
    case 2:
        return GameMode::SPECTATOR;
    }

    throw std::invalid_argument{"GameMode id is invaild"};
}

} // namespace cubed
