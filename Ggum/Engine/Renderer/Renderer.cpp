#include "EnginePch.h"
#include "Renderer.h"

namespace GG {



void GG::Renderer::Init(HWND hWnd, std::shared_ptr<GraphicsAPI> api)
{
	_api = api;
}

void Renderer::Prepare()
{
	_api->WaitDeviceIdle();
	_api->Begin();
}

void Renderer::Submit()
{
	_api->Draw();

}

void Renderer::PrepareGUI()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Renderer::SubmitGUI()
{
	ImGui::Render();
	ImDrawData* mainDrawData = ImGui::GetDrawData();
	const bool isMainWindowMinimized = !(mainDrawData->DisplaySize.x <= 0.0f || mainDrawData->DisplaySize.y <= 0.0f);

	if (isMainWindowMinimized)
	{
		_api->RenderImGui();
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Renderer::Present()
{
	_api->End();
}

void Renderer::PresentGUI()
{

}

void Renderer::OnResize(uint32 width, uint32 height)
{
	if (width == 0 || height == 0)
	{
		_api->SetMinimized(true);
	}
	else
	{
		_api->SetMinimized(false);
	}
}

void Renderer::SetPixelForDebug(uint32 row, uint32 col, uint8* color)
{
	_api->SetPixel(row, col, color);
}

}