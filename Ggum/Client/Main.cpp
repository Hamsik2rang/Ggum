#include <gg.h>

#include "TestRenderPass.h"

int main(int argc, char** argv)
{
	auto app = GG::Application::Get();
	app->AddRenderPass(std::make_shared<TestRenderPass>("TestPass 1", GG::RenderPassOrder::AfterRendereing));
	app->AddRenderPass(std::make_shared<TestRenderPass>("TestPass 2", GG::RenderPassOrder::AfterRendereing));

	app->Run();
	
	delete app;

	return 0;
}