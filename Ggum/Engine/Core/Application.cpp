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

	_renderer = std::make_unique<Renderer>();
	_renderer->Init(_window->GetWindowHandle());
}

Application::~Application()
{

}

void Application::Run()
{
	GG_TRACE("Application is Run.");
	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		_renderer->Draw();
	}
}

void Application::OnEvent(Event& e)
{
	GG_TRACE("On {0} Event", e.GetName());
}

}