#include "EnginePch.h"
#include "Renderer.h"

namespace GG {



void GG::Renderer::Init(HWND hWnd)
{
	_api = std::make_unique<GraphicsAPI>(hWnd);
	_api->Init();
}

void Renderer::Draw()
{
	_api->Draw();
	_api->WaitDeviceIdle();
}

}