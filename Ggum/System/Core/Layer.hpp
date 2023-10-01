#pragma once

#include <string>

#include "Base.hpp"
#include "Core/Event/Event.hpp"

namespace GG {

class Layer
{
public:
	Layer(const std::string& name)
		: _name{ name }
	{}

	~Layer() = default;
	
	// Application에 Layer가 추가될 때 호출됩니다
	virtual void OnAttach() {}
	// Application에서 Layer가 제거될 때 호출됩니다
	virtual void OnDetach() {}
	// Application의 매 Update Loop마다 호출됩니다
	virtual void OnUpdate(float deltaTime) {}
	// Application에서 이벤트가 발생할 때마다 호출됩니다
	virtual void OnEvent(Event& e) {}
	// Application의 매 Render Loop마다 호출됩니다
	virtual void OnRender() {}
	// Application의 GUI Render Loop마다 호출됩니다
	virtual void OnGUI() {}

protected:

	std::string _name;
};


}