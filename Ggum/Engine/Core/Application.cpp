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

	std::shared_ptr<GraphicsAPI> api = std::make_shared<GraphicsAPI>(_window->GetWindowHandle());

	_renderer = std::make_shared<Renderer>();
	_renderer->Init(api);

	_guiRenderer = std::make_unique<GUIRenderer>();
	_guiRenderer->Init(api);

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

		_renderer->Prepare();

		for (const auto& renderPass : _renderPath)
		{
			renderPass->OnUpdate(deltaTime);
			renderPass->OnRender();
		}

		_renderer->Submit();
		_renderer->Present();

		_guiRenderer->Prepare();
		_guiRenderer->Submit();
		_guiRenderer->Present();
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

void Application::AddRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	_renderPath.AddRenderPass(renderPass);
}

void Application::DeleteRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	_renderPath.DeleteRenderPass(renderPass);
}

}