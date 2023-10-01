#pragma once

#include "Base.hpp"
#include "Renderer/RenderPass.hpp"

namespace GG {


class RenderPath
{
public:
	RenderPath();
	~RenderPath();

	void AddRenderPass(std::shared_ptr<RenderPass> renderPass);
	void DeleteRenderPass(std::shared_ptr<RenderPass> renderPass);

	// RenderPass ��ȸ�� ���� Iterator ����
	[[nodiscard]] inline std::list<std::shared_ptr<RenderPass>>::iterator begin() { return _renderPasses.begin(); }
	[[nodiscard]] inline std::list<std::shared_ptr<RenderPass>>::iterator end() { return _renderPasses.end(); }

private:
	std::list<std::shared_ptr<RenderPass>> _renderPasses;
	std::list<std::shared_ptr<RenderPass>>::iterator _lastPassIterator;
};

}