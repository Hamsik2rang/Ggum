#pragma once

#include <sstream>

#include "Core/Event/Event.hpp"

namespace GG {

class MouseMovedEvent : public Event
{
public:
	MouseMovedEvent(float x, float y)
		: _xPos{ x }
		, _yPos{ y }
	{}

	inline float GetX() const { return _xPos; }
	inline float GetY() const { return _yPos; }

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseMovedEvent: " << _xPos << ", " << _yPos;

		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseMoved)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
	float _xPos;
	float _yPos;
};

class MouseScrolledEvent : public Event
{
public:
	// Horizontal Scroll(hOffset)은 일반적인 마우스 조작의 경우 입력되지 않습니다.
	MouseScrolledEvent(float vOffset, float hOffset = 0.0f)
		: _vOffset(vOffset)
		, _hOffset(hOffset)
	{}

	inline float GetVOffset() const { return _vOffset; }
	inline float GetHOffset() const { return _hOffset; }

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseScrolledEvent: " << _vOffset;

		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseScrolled)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
	float _vOffset;	// vertical
	float _hOffset;	// horizontal. Most users don't have a mouse with an horizontal wheel.

};

class MouseButtonEvent : public Event
{
public:
	inline int GetMouseButton() const { return _button; }

	EVENT_CLASS_CATEGORY(EventCategoryMouseButton | EventCategoryInput)

protected:
	int _button;

	MouseButtonEvent(int button)
		:_button(button)
	{}
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
	MouseButtonPressedEvent(int button)
		:MouseButtonEvent(button)
	{}

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << _button;

		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
	MouseButtonReleasedEvent(int button)
		:MouseButtonEvent(button)
	{}

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << _button;

		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonReleased)
};
}