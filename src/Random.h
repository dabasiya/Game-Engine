#pragma once

#include <random>

struct Random {

	static void Init() {
		s_random.seed(std::random_device()());
	}

	static float Float() {
		return (float)s_distribution(s_random) / (float)std::numeric_limits<uint32_t>::max();
	}

	static std::mt19937 s_random;
	static std::uniform_int_distribution<std::mt19937::result_type> s_distribution;
};
