#pragma once

#include "System/gg_system.h"
#include "Base.hpp"

namespace GG {

class IDrawable
{
public:
	virtual ~IDrawable() {}

	virtual void Init(std::shared_ptr<GraphicsAPI> api) = 0;
	virtual void Prepare() = 0;
	virtual void Submit() = 0;
	virtual void Present() = 0;
	//...

protected:
	std::shared_ptr<GraphicsAPI> _api = nullptr;
	//... 

};


}