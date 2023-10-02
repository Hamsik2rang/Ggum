#include "EnginePch.h"
#include "Renderer.h"

namespace GG {



void GG::Renderer::Init(HWND hWnd, std::shared_ptr<GraphicsAPI> api)
{
	_api = api;
}

void Renderer::Prepare()
{}

void Renderer::Submit()
{
	_api->Draw();
	_api->WaitDeviceIdle();
}

void Renderer::Present()
{}

}