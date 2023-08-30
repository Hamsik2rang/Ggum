#pragma once

#include <sstream>

#include "Core/Event/Event.hpp"

namespace GG {

class KeyEvent : public Event
{
public:
	inline int GetKeyCode() const { return _keyCode; }

	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
	int _keyCode;

	KeyEvent(int keyCode)
		:_keyCode{ keyCode }
	{}
};

class KeyPressedEvent : public KeyEvent
{
public:
	KeyPressedEvent(int keyCode, int repeatCount)
		: KeyEvent(keyCode)
		, _repeatCount{ repeatCount }
	{}

	inline int GetRepeatCount() const { return _repeatCount; }
	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << _keyCode << "(" << _repeatCount << "repeats)";

		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyPressed)

private:
	int _repeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
	KeyReleasedEvent(int keyCode)
		: KeyEvent(keyCode)
	{}

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << _keyCode;

		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased)
};

}