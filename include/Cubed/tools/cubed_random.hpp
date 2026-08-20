#pragma once
#include "glm/ext/vector_float3.hpp"

#include <random>
namespace cubed {

class Random {
public:
    Random();
    Random(unsigned seed);
    bool random_bool(double probability);
    std::mt19937& engine();
    unsigned seed();

    void init(unsigned seed);
    int random_int(int min, int max);
    float random_float(float min, float max);

    glm::vec3 random_direction_horizontal();

private:
    unsigned int m_seed = 0;
    std::mt19937 m_engine;
};

} // namespace cubed
