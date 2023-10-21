#pragma once

#include "Base.hpp"
#include "Renderer/Renderer.h"

#include "Renderer/RenderPath.h"

#include "System/gg_system.h"


namespace GG {

class Application
{
public:
	~Application();

	void Run();

	void OnEvent(Event& e);

	void AddRenderPass(std::shared_ptr<RenderPass> renderPass);
	void DeleteRenderPass(std::shared_ptr<RenderPass> renderPass);

	static Application* Get();

protected:
	Application(const char* title = "GG Engine", uint32 width = 1600, uint32 height = 900);

private:
	static Application* s_instance;
	std::shared_ptr<Renderer> _renderer;

	std::unique_ptr<Window> _window;
	RenderPath _renderPath;
	
	Timer _timer;
};
}