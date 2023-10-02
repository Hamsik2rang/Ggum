#include "EnginePch.h"

#include "Renderer/GUIRenderer.h"


namespace GG {

void GUIRenderer::Init(HWND hWnd, std::shared_ptr<GraphicsAPI> api)
{
	_api = api;
}

void GUIRenderer::Prepare()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

void GUIRenderer::Submit()
{
	bool showDemoWindow = true;
	ImGui::ShowDemoWindow(&showDemoWindow);
	
	ImGui::Render();
	ImDrawData* mainDrawData = ImGui::GetDrawData();
	const bool isMainWindowMinimized = (mainDrawData->DisplaySize.x <= 0.0f || mainDrawData->DisplaySize.y <= 0.0f);

	if (!isMainWindowMinimized)
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

void GUIRenderer::Present()
{
	ImDrawData* mainDrawData = ImGui::GetDrawData();
	const bool isMainWindowMinimized = (mainDrawData->DisplaySize.x <= 0.0f || mainDrawData->DisplaySize.y <= 0.0f);

	if (!isMainWindowMinimized)
	{
		_api->PresentImGui();
	}

	_api->WaitDeviceIdle();
}

}