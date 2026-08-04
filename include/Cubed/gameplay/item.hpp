#pragma once
#include <cstdint>
#include <string>
namespace Cubed {
using ItemID = uint16_t;

struct ItemData {
    ItemID id = 0;
    std::string name;
    std::string description;
    std::string path;
};

} // namespace Cubed