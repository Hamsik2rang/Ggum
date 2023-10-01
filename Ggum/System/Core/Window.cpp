#include "SystemPch.h"
#include "Window.h"

#include "Core/Log.h"
#include "Utility/Utility.hpp"

#include "Platform/Win32.h"

namespace GG {

GG::Window::Window(const WindowProperty& prop)
	: _data{}
	, _hWnd{ nullptr }
{
	_data.title = prop.title;
	_data.width = prop.width;
	_data.height = prop.height;
	_data.isVerticalSync = false;

	GG_INFO("Creating Window {0} ({1}, {2})", _data.title, _data.width, _data.height);
	ATOM result = register_window_class(_data.title, window_process);

	GG_INFO("register window class result is {0}", result);
	if (!result)
	{
		GG_CRITICAL("Can't Initialize Main Window!");
	}
	
	_hWnd = init_instance(_data.title, _data.width, _data.height, reinterpret_cast<LONG_PTR>(&_data));
	if (!_hWnd)
	{
		GG_CRITICAL("Can't Initialize Window Handle!");
	}

	// Create Graphcis Context

	show_window(_hWnd);

	::SetWindowLongPtr(_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&_data));

	GG_INFO("Window {0} is created.");
}

void Window::OnUpdate()
{

}

}