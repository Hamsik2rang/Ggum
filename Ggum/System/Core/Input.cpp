#include "SystemPch.h"

#include "Input.h"
#include "Core/Log.h"


namespace GG {

Axis Input::s_axis{};
uint64 Input::s_keyMap[5]{};

bool Input::IsButtonDown(MouseCode mouseButton)
{
    GG_ASSERT(mouseButton < 256, "Invalid button");

    return s_keyMap[mouseButton / s_bitCount] & BIT_UINT64(mouseButton % s_bitCount);
}

bool Input::IsButtonUp(MouseCode mouseButton)
{
    GG_ASSERT(mouseButton < 256, "Invalid button");

    return !(s_keyMap[mouseButton / s_bitCount] & BIT_UINT64(mouseButton % s_bitCount));
}

bool Input::IsKeyDown(KeyCode keyCode)
{
    GG_ASSERT(keyCode < 256, "invalid keycode");

    return s_keyMap[keyCode / s_bitCount] & BIT_UINT64(keyCode % s_bitCount);
}

bool Input::IsKeyUp(KeyCode keyCode)
{
    GG_ASSERT(keyCode < 256, "invalid keycode");

    return !(s_keyMap[keyCode / s_bitCount] & BIT_UINT64(keyCode % s_bitCount));
}

void Input::SetKeyDown(KeyCode keyCode)
{
    s_keyMap[keyCode / s_bitCount] |= BIT_UINT64(keyCode % s_bitCount);
}

void Input::SetKeyUp(KeyCode keyCode)
{
    s_keyMap[keyCode / s_bitCount] &= ~BIT_UINT64(keyCode % s_bitCount);
}

void Input::SetButtonDown(MouseCode mouseButton)
{
    s_keyMap[mouseButton / s_bitCount] |= BIT_UINT64(mouseButton % s_bitCount);
}

void Input::SetButtonUp(MouseCode mouseButton)
{
    s_keyMap[mouseButton / s_bitCount] &= ~BIT_UINT64(mouseButton % s_bitCount);
}

}