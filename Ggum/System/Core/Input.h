#pragma once

#include "Base.hpp"
#include "Core/InputCode.hpp"
#include "Core/Log.h"

namespace GG {

struct Axis
{
	int horizontal = 0;
	int vertical = 0;
};

class Input
{
public:
	inline static bool IsButtonDown(MouseCode mouseButton) { GG_ASSERT(mouseButton < 256, "Invalid button"); return s_buttonMap & BIT_UINT64(mouseButton % s_bitCount); }
	inline static bool IsButtonUp(MouseCode mouseButton) { GG_ASSERT(mouseButton < 256, "Invalid button"); return !(s_buttonMap & BIT_UINT64(mouseButton % s_bitCount)); }
	inline static bool IsKeyDown(KeyCode keyCode) { GG_ASSERT(keyCode < 256, "invalid keycode"); return s_keyMap[keyCode / s_bitCount] & BIT_UINT64(keyCode % s_bitCount); }
	inline static bool IsKeyUp(KeyCode keyCode) { GG_ASSERT(keyCode < 256, "invalid keycode"); return !(s_keyMap[keyCode / s_bitCount] & BIT_UINT64(keyCode % s_bitCount)); }

	inline static void SetKeyDown(KeyCode keyCode) { GG_ASSERT(keyCode < 256, "invalid keycode"); s_keyMap[keyCode / s_bitCount] |= BIT_UINT64(keyCode % s_bitCount); }
	inline static void SetKeyUp(KeyCode keyCode) { GG_ASSERT(keyCode < 256, "invalid keycode"); s_keyMap[keyCode / s_bitCount] &= ~BIT_UINT64(keyCode % s_bitCount); }
	inline static void SetButtonDown(MouseCode  mouseButton) { GG_ASSERT(mouseButton < 256, "Invalid button"); s_buttonMap |= BIT_UINT64(mouseButton % s_bitCount); }
	inline static void SetButtonUp(MouseCode  mouseButton) { GG_ASSERT(mouseButton < 256, "Invalid button"); s_buttonMap &= ~BIT_UINT64(mouseButton % s_bitCount); }

	inline static Axis GetAxis() { return s_axis; };

private:
	static Axis s_axis;
	static uint64 s_keyMap[5];
	static uint64 s_buttonMap;
	static const int s_bitCount = 64;

};

}