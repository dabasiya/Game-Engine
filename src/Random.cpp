#include "Random.h"

std::mt19937 Random::s_random;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_distribution;
