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

void RenderPath::AddRenderPass(std::shared_ptr<RenderPass> renderPass)
{
	_renderPasses.push_back(renderPass);

	std::stable_sort(_renderPasses.begin(), _renderPasses.end(), [](const std::shared_ptr<RenderPass>& lPass, const std::shared_ptr<RenderPass>& rPass)->bool {
		return lPass->_order < rPass->_order;

		});
}


void RenderPath::DeleteRenderPass(std::shared_ptr<RenderPass> renderPass)
{

}


}