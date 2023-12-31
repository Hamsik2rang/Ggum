#pragma once

#include "System/gg_system.h"
#include "Base.hpp"

namespace GG {

class IDrawable
{
public:
	virtual ~IDrawable() {}

	virtual void Init(HWND hWnd, std::shared_ptr<GraphicsAPI> api) = 0;
	virtual void Prepare() = 0;
	virtual void Submit() = 0;
	virtual void Present() = 0;
	virtual void PrepareGUI() = 0;
	virtual void SubmitGUI() = 0;
	virtual void PresentGUI() = 0;
	//...

	virtual void OnResize(uint32 width, uint32 height) = 0;

	virtual void SetPixelForDebug(uint32 row, uint32 col, uint8* color) = 0;

protected:
	std::shared_ptr<GraphicsAPI> _api = nullptr;
	//... 

};


}