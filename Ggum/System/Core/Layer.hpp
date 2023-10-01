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
	
	// Application�� Layer�� �߰��� �� ȣ��˴ϴ�
	virtual void OnAttach() {}
	// Application���� Layer�� ���ŵ� �� ȣ��˴ϴ�
	virtual void OnDetach() {}
	// Application�� �� Update Loop���� ȣ��˴ϴ�
	virtual void OnUpdate(float deltaTime) {}
	// Application���� �̺�Ʈ�� �߻��� ������ ȣ��˴ϴ�
	virtual void OnEvent(Event& e) {}
	// Application�� �� Render Loop���� ȣ��˴ϴ�
	virtual void OnRender() {}
	// Application�� GUI Render Loop���� ȣ��˴ϴ�
	virtual void OnGUI() {}

protected:

	std::string _name;
};


}