#include "EnginePch.h"
#include "Renderer.h"

namespace GG {



void GG::Renderer::Init(std::shared_ptr<GraphicsAPI> api)
{
	_api = api;
}

void Renderer::Submit()
{
	_api->Draw();
	_api->WaitDeviceIdle();
}

}