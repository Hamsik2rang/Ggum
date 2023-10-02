#pragma once

#include "Renderer/Drawable.hpp"

namespace GG {

class GUIRenderer : public IDrawable
{
public:
	virtual void Init(std::shared_ptr<GraphicsAPI> api);
	virtual void Prepare() override;
	virtual void Submit() override;
	virtual void Present() override;

private:
	//...
};

}