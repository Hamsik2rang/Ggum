#pragma once

#include <random>
#include <numeric>

#include "Base.hpp"

namespace GG {

class Random
{
public:
	static void Init()
	{
		s_mersenne.seed(std::random_device()());
	}


private:
	static std::mt19937_64 s_mersenne;
};

}