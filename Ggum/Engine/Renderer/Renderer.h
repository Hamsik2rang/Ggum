#pragma once

#include "System/gg_system.h"

namespace GG {

class Renderer
{
public:
	void Init(HWND hWnd);
	void Draw(/* ... */);
	//...

private:
	std::unique_ptr<GraphicsAPI> _api;
	//... 
};


}