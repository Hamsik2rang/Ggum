#pragma once

#include "Base.hpp"

#include <string>
#include <iostream>

namespace GG {

enum class eEventType
{
	None = 0,

	KeyPressed,
	KeyReleased,

	MouseMoved,
	MouseScrolled,
	MouseButtonPressed,
	MouseButtonReleased,

	WindowClosed,
	WindowResized,
	WindowFocused,
	WindowLostFocused,
	WindowMoved,

	AppTick,
	AppUpdate,
	AppRender,
};

enum EventCategory : uint16
{
	None = 0,
	EventCategoryApplication	= BIT(0),
	EventCategoryInput			= BIT(1),
	EventCategoryKeyboard		= BIT(2),
	EventCategoryMouse			= BIT(3),
	EventCategoryMouseButton	= BIT(4),
};

#define EVENT_CLASS_TYPE(type) static eEventType GetStaticType() { return eEventType::##type; } \
								virtual eEventType GetEventType() const override { return GetStaticType(); } \
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual uint16 GetCategoryFlags() const override { return category; }

class Event
{
	friend class EventDispatcher;

public:
	virtual eEventType GetEventType() const = 0;
	virtual const char* GetName() const = 0;
	virtual uint16 GetCategoryFlags() const = 0;
	virtual std::string ToString() const { return std::string(GetName()); }

	inline bool IsHandled() const { return _isHandled; }
	inline bool IsInCategory(EventCategory category) { return (GetCategoryFlags() & category); }

protected:
	bool _isHandled;
};

class EventDispatcher
{
	template <typename T>
	using EventFunc = std::function<bool(T&)>;

public:
	EventDispatcher(Event& event)
		: _event{ event }
	{}

	template<typename T>
	bool Dispatch(EventFunc<T> func)
	{
		if (_event.GetEventType() == T::GetStaticType())
		{
			_event._isHandled = func(*(T*)&_event);
			return true;
		}

		return false;
	}

private:
	Event& _event;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
	return os << e.ToString();
}

}