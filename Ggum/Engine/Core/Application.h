#pragma once

#include "Base.hpp"

namespace GG {

class GG_API Application
{
public:
	~Application();
	void Run();

	static Application* Get();
protected:
	Application();

private:
	static Application* s_instance;
};
}