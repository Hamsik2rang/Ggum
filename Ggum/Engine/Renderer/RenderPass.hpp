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

	// Application에 RenderPass가 추가될 때 호출됩니다
	virtual void OnAttach() {}
	// Application에서 RenderPass가 제거될 때 호출됩니다
	virtual void OnDetach() {}
	// Application의 매 Update Loop마다 호출됩니다
	virtual void OnUpdate(float deltaTime) {}
	// Application에서 이벤트가 발생할 때마다 호출됩니다
	virtual void OnEvent(Event& e) {}
	// Application의 매 Render Loop마다 호출됩니다
	virtual void OnRender() {}
	// Application의 GUI Render Loop마다 호출됩니다
	virtual void OnGUI() {}

	inline RenderPassOrder GetOrder() { return _order; }
	inline std::string GetName() { return _name; }

protected:
	const Renderer* const _renderer;
	std::string _name;
	RenderPassOrder _order;
};


}