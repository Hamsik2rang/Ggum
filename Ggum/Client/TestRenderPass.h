#pragma once

#include "gg.h"

class TestRenderPass : public GG::RenderPass
{
	using Base = GG::RenderPass;

public:
	TestRenderPass(std::string passName, GG::RenderPassOrder order);

	// Application에 RenderPass가 추가될 때 호출됩니다
	virtual void OnAttach();
	// Application에서 RenderPass가 제거될 때 호출됩니다
	virtual void OnDetach();
	// Application의 매 Update Loop마다 호출됩니다
	virtual void OnUpdate(float deltaTime);
	// Application에서 이벤트가 발생할 때마다 호출됩니다
	virtual void OnEvent(GG::Event& e);
	// Application의 매 Render Loop마다 호출됩니다
	virtual void OnRender();
	// Application의 GUI Render Loop마다 호출됩니다
	virtual void OnGUI();

private:
	uint8 _color[4];
};