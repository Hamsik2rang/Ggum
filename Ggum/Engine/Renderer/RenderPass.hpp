#pragma once

#include <string>

#include "Base.hpp"
#include "Renderer/Renderer.h"

#include "System/gg_system.h"

namespace GG {

enum RenderPassOrder
{
	BeforeRendering = 0,

	Skybox = 200,

	Opaque = 250,

	Transparent = 300,

	BeforePostProcess = 500,

	AfterPostProcess = 850,

	AfterRendereing = 990
};


class RenderPass
{
public:
	friend RenderPath;

	RenderPass(const std::string& name, const Renderer* const renderer, RenderPassOrder renderingOrder)
		: _name{ name }
		, _renderer{ renderer }
		, _order{ renderingOrder }
	{}

	~RenderPass() = default;

	// Application�� RenderPass�� �߰��� �� ȣ��˴ϴ�
	virtual void OnAttach() {}
	// Application���� RenderPass�� ���ŵ� �� ȣ��˴ϴ�
	virtual void OnDetach() {}
	// Application�� �� Update Loop���� ȣ��˴ϴ�
	virtual void OnUpdate(float deltaTime) {}
	// Application���� �̺�Ʈ�� �߻��� ������ ȣ��˴ϴ�
	virtual void OnEvent(Event& e) {}
	// Application�� �� Render Loop���� ȣ��˴ϴ�
	virtual void OnRender() {}
	// Application�� GUI Render Loop���� ȣ��˴ϴ�
	virtual void OnGUI() {}

	inline RenderPassOrder GetOrder() { return _order; }
	inline std::string GetName() { return _name; }

protected:
	const Renderer* const _renderer;
	std::string _name;
	RenderPassOrder _order;
};


}