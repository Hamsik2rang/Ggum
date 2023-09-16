#pragma once

#include <Windows.h>
#include <string>

#include "Base.hpp"

namespace GG {

using WindowsCallback = WNDPROC;

ATOM register_window_class(const std::string& title, WindowsCallback callback);
HWND init_instance(const std::string& title, uint32 width, uint32 height, LONG_PTR userData);
LRESULT window_process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void show_window(HWND hWnd);
DWORD get_display_frequency();


}