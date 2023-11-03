#pragma once

#include "Base.hpp"
#include "Core/InputCode.hpp"

namespace GG {

struct Axis
{
	int horizontal = 0;
	int vertical = 0;
};

class Input
{
public:
	static bool IsButtonDown(MouseCode mouseButton);
	static bool IsButtonUp(MouseCode mouseButton);
	static bool IsKeyDown(KeyCode keyCode);
	static bool IsKeyUp(KeyCode keyCode);

	static void SetKeyDown(KeyCode keyCode);
	static void SetKeyUp(KeyCode keyCode);
	static void SetButtonDown(MouseCode  mouseButton);
	static void SetButtonUp(MouseCode  mouseButton);

	inline static Axis GetAxis() { return s_axis; };

private:
	static Axis s_axis;
	static uint64 s_keyMap[5];
	static const int s_bitCount = 64;

};

}