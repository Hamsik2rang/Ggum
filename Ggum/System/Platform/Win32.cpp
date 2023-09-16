#include "SystemPch.h"
#include "Win32.h"

#include "Utility.hpp"
#include "Core/Window.h"
#include "Core/Event/KeyEvent.hpp"
#include "Core/Event/MouseEvent.hpp"
#include "Core/Event/ApplicationEvent.hpp"

namespace GG {

ATOM register_window_class(const std::string& title, WindowsCallback callback)
{
	WNDCLASSEXW wcex{};

	std::wstring wTitle = Utility::string_to_wstring(title);
	HINSTANCE hInst = GetModuleHandle(nullptr);

	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_CLASSDC;
	wcex.lpfnWndProc = callback;
	wcex.cbClsExtra = 0;
	// 이벤트 핸들링을 위한 Userdata를 등록하기 위해 LONG_PTR 하나 만큼의 여분 메모리를 지정합니다.
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = hInst;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = wTitle.c_str();
	wcex.hIconSm = LoadIcon(hInst, nullptr);

	return ::RegisterClassExW(&wcex);
}

HWND init_instance(const std::string& title, uint32 width, uint32 height, LONG_PTR userData)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	std::wstring wTitle = Utility::string_to_wstring(title);
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	int dwFrameX = GetSystemMetrics(SM_CXFRAME);
	int dwFrameY = GetSystemMetrics(SM_CYFRAME);
	int dwCaptionY = GetSystemMetrics(SM_CYCAPTION);

	HWND hWnd = nullptr;
	hWnd = CreateWindowW(wTitle.c_str(),
		wTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		(DWORD)width + (dwFrameX * 2),
		(DWORD)height + (dwFrameY * 2) + dwCaptionY,
		nullptr,
		nullptr,
		hInst,
		0);

	return hWnd;
}

LRESULT window_process(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window::WindowData* data = reinterpret_cast<Window::WindowData*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));

	if (nullptr == data)
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	switch (msg)
	{
	case WM_KEYDOWN:
	{
		int repeatCount = LOWORD(lParam);
		KeyPressedEvent e(static_cast<int>(wParam), repeatCount);
		data->eventCallback(e);
		break;
	}
	case WM_KEYUP:
	{
		KeyReleasedEvent e(static_cast<int>(wParam));

		data->eventCallback(e);
		break;
	}
	case WM_MOUSEMOVE:
	{
		POINT cursorPos{};
		::GetCursorPos(&cursorPos);
		::ScreenToClient(hWnd, &cursorPos);

		MouseMovedEvent e(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y));
		
		data->eventCallback(e);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		short z = GET_WHEEL_DELTA_WPARAM(wParam);
		MouseScrolledEvent e(z, 0.0f);
		
		data->eventCallback(e);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		int pressedButton = static_cast<int>(wParam);
		if		(msg == WM_LBUTTONDOWN);//pressedButton =
		else if (msg == WM_MBUTTONDOWN);
		else if (msg == WM_RBUTTONDOWN);

		MouseButtonPressedEvent e(pressedButton);

		data->eventCallback(e);
		break;
	}
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	{
		int releasedButton = static_cast<int>(wParam);
		if		(msg == WM_LBUTTONUP);
		else if (msg == WM_MBUTTONUP);
		else if (msg == WM_RBUTTONUP);

		MouseButtonReleasedEvent e(releasedButton);

		data->eventCallback(e);
		break;
	}
	case WM_SIZE:
	{
		RECT rcRect{};
		::GetClientRect(hWnd, &rcRect);
		::ScreenToClient(hWnd, (POINT*)&rcRect.left);
		::ScreenToClient(hWnd, (POINT*)&rcRect.right);

		uint32 width = rcRect.right - rcRect.left;
		uint32 height = rcRect.bottom - rcRect.top;

		WindowResizeEvent e(width, height);
		data->width = width;
		data->height = height;
		data->eventCallback(e);
		break;
	}
	case WM_CLOSE:
	{
		WindowCloseEvent e;
		data->eventCallback(e);
	}
	[[fallthrough]]
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	default: 
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}
	}

	return 0;
}

void show_window(HWND hWnd)
{
	::ShowWindow(hWnd, SW_SHOWNORMAL);
	::UpdateWindow(hWnd);
}

DWORD get_display_frequency()
{
	DEVMODEA devMode{};
	if (EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &devMode))
	{
		return devMode.dmDisplayFrequency;
	}

	return 0;
}
}