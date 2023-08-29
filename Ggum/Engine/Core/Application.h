#pragma once

#include "Base.hpp"

#include "System/gg_system.h"

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