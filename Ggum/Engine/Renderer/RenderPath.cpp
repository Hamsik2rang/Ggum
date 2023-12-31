#include "EnginePch.h"

#include "RenderPath.h"


namespace GG {


RenderPath::RenderPath()
{

}

RenderPath::~RenderPath()
{
	_renderPasses.clear();
}

void RenderPath::SetRenderer(std::shared_ptr<Renderer> renderer)
{
	_renderer = renderer;
}

void RenderPath::Clear()
{
	_renderPasses.clear();
}

void RenderPath::AddRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	renderPass->_renderer = _renderer;
	_renderPasses.push_back(renderPass);

	std::stable_sort(_renderPasses.begin(), _renderPasses.end(), [](const std::shared_ptr<RenderPass>& lPass, const std::shared_ptr<RenderPass>& rPass)->bool {
		return lPass->GetOrder() < rPass->GetOrder();

		});
}


void RenderPath::DeleteRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	if (std::find(_renderPasses.begin(), _renderPasses.end(), renderPass) != _renderPasses.end())
	{
		_renderPasses.remove(renderPass);
	}
}


}