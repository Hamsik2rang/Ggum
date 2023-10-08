#pragma once

#include <sstream>

#include "Core/Event/Event.hpp"

namespace GG {

class WindowResizeEvent : public Event
{
private:
	uint32 _width;
	uint32 _height;

public:
	WindowResizeEvent(uint32_t width, uint32_t height)
		: _width(width)
		, _height(height)
	{}

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "WindowResizeEvent: " << _width << ", " << _height;

		return ss.str();
	}

	uint32 GetWidth() { return _width; }
	uint32 GetHeight() { return _height; }

	EVENT_CLASS_TYPE(WindowResized)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowCloseEvent : public Event
{
public:
	WindowCloseEvent() {}

	EVENT_CLASS_TYPE(WindowClosed)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppTickEvent : public Event
{
public:
	AppTickEvent() {}

	EVENT_CLASS_TYPE(AppTick)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppUpdateEvent : public Event
{
public:
	AppUpdateEvent() {}

	EVENT_CLASS_TYPE(AppUpdate)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class AppRenderEvent : public Event
{
public:
	AppRenderEvent() {}

	EVENT_CLASS_TYPE(AppRender)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

}