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

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

	static double Real(double minValue = std::numeric_limits<double>::min(), double maxValue = std::numeric_limits<double>::max())
	{
		std::uniform_real_distribution<double> dis(minValue, maxValue);
		double gen = dis(s_mersenne);

		return gen;
	}

	static int64 Int(int64 minValue = std::numeric_limits<int64>::min(), int64 maxValue = std::numeric_limits<int64>::max())
	{
		std::uniform_int_distribution<int64> dis(minValue, maxValue);
		int64_t gen = dis(s_mersenne);

		return gen;
	}

	static uint64 UInt(uint64 minValue = std::numeric_limits<uint64>::min(), uint64 maxValue = std::numeric_limits<uint64>::max())
	{
		std::uniform_int_distribution<uint64> dis(minValue, maxValue);
		uint64_t gen = dis(s_mersenne);

		return gen;
	}

#pragma pop_macro("max")
#pragma pop_macro("min")

private:
	static std::mt19937_64 s_mersenne;
};

}