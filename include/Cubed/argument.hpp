#pragma once
#include <string>
namespace Cubed {
struct Argument {
    bool is_client = false;
    int port = 25530;
    std::string ip{"127.0.0.1"};
    std::string player{"Unknown"};
    bool debug_on = true;
};
} // namespace Cubed