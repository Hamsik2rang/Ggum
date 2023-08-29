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

}

Application::~Application()
{

}

void Application::Run()
{
	std::cout << "Hello GG Renderer!" << std::endl;
}

}