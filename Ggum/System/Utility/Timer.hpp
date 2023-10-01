#pragma once

#include <chrono>

#include "Base.hpp"

namespace GG {

class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	inline void Init() { _timePoint = std::chrono::high_resolution_clock::now(); }
	inline void Start() { Init(); }
	// 초 단위 타이머
	inline float Elapsed() { return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - _timePoint).count() * 0.001f * 0.001f * 0.001f; }
	// 밀리초 단위 타이머
	inline float ElapsedMills() { return Elapsed() * 1000.0f; }

private:

	std::chrono::time_point<std::chrono::high_resolution_clock> _timePoint;
};

}