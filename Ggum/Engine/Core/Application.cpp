#include "EnginePch.h"

#include "Application.h"

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

Application::Application()
{
	GG::Log::Init();
}

Application::~Application()
{

}

void Application::Run()
{
	GG_TRACE("Application is Run.");
}

}