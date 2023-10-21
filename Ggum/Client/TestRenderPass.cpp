#include "TestRenderPass.h"


using namespace GG;

TestRenderPass::TestRenderPass(std::string passName, GG::RenderPassOrder order)
	: Base(passName, order)
{}

void TestRenderPass::OnAttach()
{

}

void TestRenderPass::OnDetach()
{

}

void TestRenderPass::OnUpdate(float deltaTime)
{

}

void TestRenderPass::OnEvent(GG::Event& e)
{

}

void TestRenderPass::OnRender()
{
	for (int i = 0; i < 720; i++)
	{
		for (int j = 0; j < 1280; j++)
		{
			_renderer->SetPixelForDebug(i, j, _color);
		}
	}
}

void TestRenderPass::OnGUI()
{
	static bool showDemoWindow = false;
	static ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImGuiIO& io = ImGui::GetIO();
	if (showDemoWindow)
	{
		ImGui::ShowDemoWindow(&showDemoWindow);
	}
	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin(_name.c_str());                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is %s Panel", _name.c_str());                   // Display some text (you can use a format strings too)

	ImGui::ColorEdit3("clear color", (float*)&clearColor);  // Edit 3 floats representing a color
	_color[0] = clearColor.x * 255;
	_color[1] = clearColor.y * 255;
	_color[2] = clearColor.z * 255;
	_color[3] = clearColor.w * 255;


	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}
