#pragma once

#include "Renderer/Drawable.hpp"

namespace GG {

class Renderer : public IDrawable
{
public:
	virtual void Init(HWND hWnd, std::shared_ptr<GraphicsAPI> api) override;
	virtual void Prepare() override;
	virtual void Submit() override;
	virtual void PrepareGUI() override;
	virtual void SubmitGUI() override;
	virtual void Present() override;
	virtual void PresentGUI() override;
	//...
	virtual void OnResize(uint32 width, uint32 height) override;

	virtual void SetPixelForDebug(uint32 row, uint32 col, uint8* color) override;

private:
	//...
};


}