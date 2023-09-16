#pragma once

#include "Base.hpp"

#include "Core/Event/Event.hpp"

#include <string>
#include <functional>

namespace GG {

struct WindowProperty
{
	std::string title;
	uint32 width;
	uint32 height;

	WindowProperty(const std::string& title = "GG Engine", uint32 width = 1600, uint32 height = 1050)
		: title(title)
		, width(width)
		, height(height)
	{}
};
	
class Window
{
public:
	using EventCallbackFunc = std::function<void(Event&)>;

	struct WindowData
	{
		std::string title;
		uint32 width;
		uint32 height;
		bool isVerticalSync;

		EventCallbackFunc eventCallback;
	};

	Window() = delete;
	Window(const WindowProperty& prop);

	~Window() = default;

	void OnUpdate();
	void GetCursorPos(uint32_t& outXPos, uint32_t& outYPos) {}

	inline uint32_t GetWidth() const { return _data.width; }
	inline uint32_t GetHeight() const { return _data.height; }
	inline void SetEventCallback(const EventCallbackFunc& callback) { _data.eventCallback = callback; }

	inline void SetVSync(bool isEnabled) { _data.isVerticalSync = isEnabled; }
	inline bool IsVSync() const { return _data.isVerticalSync; }
	HWND GetWindowHandle() const { return _hWnd; }

private:
	HWND _hWnd;

	WindowData _data;
};
}