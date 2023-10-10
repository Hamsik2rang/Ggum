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

	std::shared_ptr<GraphicsAPI> api = std::make_shared<GraphicsAPI>(_window->GetWindowHandle(), width, height);
	api->Init();

	_renderer = std::make_shared<Renderer>();
	_renderer->Init(_window->GetWindowHandle(), api);

	_renderPath.SetRenderer(_renderer);

	_timer.Init();
}

Application::~Application()
{
	_renderPath.Clear();
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



		// Rendering---------------------
		_renderer->Prepare();

		for (const auto& renderPass : _renderPath)
		{
			renderPass->OnUpdate(deltaTime);
			renderPass->OnRender();
		}

		_renderer->Submit();
		// Rendering---------------------
		
		// GUI Rendering-----------------
		_renderer->PrepareGUI();

		for (const auto& renderPass : _renderPath)
		{
			renderPass->OnGUI();
		}
		_renderer->SubmitGUI();
		// GUI Rendering-----------------
		_renderer->Present();


		// Window Update-----------------
		_window->OnUpdate();
		// Window Update-----------------

	}
}

void Application::OnEvent(Event& e)
{
	if (e.GetEventType() == eEventType::WindowResized)
	{
		WindowResizeEvent* resizeEvent = static_cast<WindowResizeEvent*>(&e);
		_renderer->OnResize(resizeEvent->GetWidth(), resizeEvent->GetHeight());
		return;
	}

	for (const auto& renderPass : _renderPath)
	{
		renderPass->OnEvent(e);
		if (e.IsHandled())
		{
			break;
		}
	}
}

void Application::AddRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	_renderPath.AddRenderPass(renderPass);
}

void Application::DeleteRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	_renderPath.DeleteRenderPass(renderPass);
}

}