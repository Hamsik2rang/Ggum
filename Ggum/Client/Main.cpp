#include <gg.h>

int main(int argc, char** argv)
{
	auto app = GG::Application::Get();

	app->Run();
	
	delete app;

	return 0;
}