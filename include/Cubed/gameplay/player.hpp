#pragma once
#include <stdexcept>
#include <utility>
namespace Cubed {
enum class Gait { STOP = 0, WALK = 1, RUN = 2 };
constexpr int get_gait_id(Gait gait) { return std::to_underlying(gait); }

inline Gait get_gait_from_id(int id) {
    switch (id) {
    case std::to_underlying(Gait::STOP):
        return Gait::STOP;
    case std::to_underlying(Gait::WALK):
        return Gait::WALK;
    case std::to_underlying(Gait::RUN):
        return Gait::RUN;
    default:
        throw std::runtime_error("Unknown Gait");
    }
}

} // namespace Cubed
