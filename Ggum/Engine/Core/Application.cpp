#include "EnginePch.h"

#include "Application.h"

#include <memory>
#include <functional>

namespace GG {

Application* Application::s_instance = nullptr;

Application* Application::Get()
{
	if (nullptr == s_instance)
	{
		s_instance = new Application;
	}

	return s_instance;
}

Application::Application(const char* title, uint32 width, uint32 height)
{
	GG::Log::Init();

	WindowProperty prop(title, width, height);

	_window = std::make_unique<Window>(prop);
	_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

	_renderer = new Renderer();
	_renderer->Init(_window->GetWindowHandle());

	_timer.Init();
}

Application::~Application()
{

}

void Application::Run()
{
	GG_TRACE("Application is Run.");
	MSG msg{};

	_timer.Start();
	float lastTime = 0.0f;

	while (msg.message != WM_QUIT)
	{
		float curTime = _timer.Elapsed();
		float deltaTime = curTime - lastTime;
		lastTime = curTime;

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		for (const auto& layer : _renderPath)
		{
			layer->OnUpdate(deltaTime);
			layer->OnRender();
		}

		_renderer->Draw();

		for (const auto& layer : _renderPath)
		{
			layer->OnGUI();
		}
	}
}

void Application::OnEvent(Event& e)
{
	for (const auto& renderPass : _renderPath)
	{
		renderPass->OnEvent(e);
		if (e.IsHandled())
		{
			break;
		}
	}
}

}