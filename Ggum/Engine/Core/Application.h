#pragma once

#include "Base.hpp"
#include "Renderer/Renderer.h"

#include "System/gg_system.h"

namespace GG {

class GG_API Application
{
public:
	~Application();
	void Run();

	void OnEvent(Event& e);

	static Application* Get();
protected:
	Application(const char* title = "GG Engine", uint32 width = 1600, uint32 height = 1050);

private:
	static Application* s_instance;
	std::unique_ptr<Window> _window;
	std::unique_ptr<Renderer> _renderer;
};
}