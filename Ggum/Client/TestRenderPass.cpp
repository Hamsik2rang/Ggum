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
	static uint8 r = 0, g = 0, b = 0;
	r += 2;
	g += 3;
	b += 5;
	uint8 color[]{ r, g, b, 255 };
	drawG(200, 350, color);
	drawG(200, 650, color);
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

void TestRenderPass::drawG(uint32 row, uint32 col, uint8* color)
{
	for (uint32 i = row; i < row + 50; i++)
	{
		for (uint32 j = col; j < col + 200; j++)
		{
			_renderer->SetPixelForDebug(i, j, color);
		}
	}
	for (uint32 i = row + 50; i < row + 250; i++)
	{
		for (uint32 j = col; j < col + 50; j++)
		{
			_renderer->SetPixelForDebug(i, j, color);
		}
	}
	for (uint32 i = row + 200; i < row + 250; i++)
	{
		for (uint32 j = col + 50; j < col + 200; j++)
		{
			_renderer->SetPixelForDebug(i, j, color);
		}
	}
	for (uint32 i = row + 100; i < row + 200; i++)
	{
		for (uint32 j = col + 150; j < col + 200; j++)
		{
			_renderer->SetPixelForDebug(i, j, color);
		}
	}
	for (uint32 i = row + 100; i < row + 150; i++)
	{
		for (uint32 j = col + 100; j < col + 150; j++)
		{
			_renderer->SetPixelForDebug(i, j, color);
		}
	}
}
