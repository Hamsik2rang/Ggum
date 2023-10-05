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

void TestRenderPass::OnEvent(GG::Event & e)
{

}

void TestRenderPass::OnRender()
{

}

void TestRenderPass::OnGUI()
{
	bool showDemoWindow = true;
	ImGui::ShowDemoWindow(&showDemoWindow);
}
