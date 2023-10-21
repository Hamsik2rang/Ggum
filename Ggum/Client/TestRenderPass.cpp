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
	static uint8 t0 = 0, t1 = 0, t2 = 0;
	t0 += 2;
	t1 += 3;
	t2 += 5;
	uint8 color[]{ t0, t1, t2, 255 };
	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 1000; j++)
		{
			_renderer->SetPixelForDebug(i, j, color);
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
	ImGui::Checkbox("Demo Window", &showDemoWindow);        // Edit bools storing our window open/close state

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clearColor);  // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	{
		counter++;
	}
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}
