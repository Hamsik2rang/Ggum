#pragma once

#include "gg.h"

class TestRenderPass : public GG::RenderPass
{
	using Base = GG::RenderPass;

public:
	TestRenderPass(std::string passName, GG::RenderPassOrder order);

	// Application�� RenderPass�� �߰��� �� ȣ��˴ϴ�
	virtual void OnAttach();
	// Application���� RenderPass�� ���ŵ� �� ȣ��˴ϴ�
	virtual void OnDetach();
	// Application�� �� Update Loop���� ȣ��˴ϴ�
	virtual void OnUpdate(float deltaTime);
	// Application���� �̺�Ʈ�� �߻��� ������ ȣ��˴ϴ�
	virtual void OnEvent(GG::Event& e);
	// Application�� �� Render Loop���� ȣ��˴ϴ�
	virtual void OnRender();
	// Application�� GUI Render Loop���� ȣ��˴ϴ�
	virtual void OnGUI();

private:
	uint8 _color[4];
};