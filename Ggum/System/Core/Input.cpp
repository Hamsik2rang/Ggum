#include "SystemPch.h"

#include "Input.h"
#include "Core/Log.h"


namespace GG {

Axis Input::s_axis{};
uint64 Input::s_keyMap[5]{};
uint64 Input::s_buttonMap;
}