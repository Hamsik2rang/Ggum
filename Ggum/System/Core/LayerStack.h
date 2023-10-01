#pragma once

#include <list>

#include "Base.hpp"
#include "Core/Layer.hpp"

namespace GG {

class LayerStack
{
public:
	LayerStack();
	~LayerStack();

	void PushLayer(std::shared_ptr<Layer> layer);
	void PushOverlay(std::shared_ptr<Layer> layer);
	void PopLayer(std::shared_ptr<Layer> layer);
	void PopOverlay(std::shared_ptr<Layer> layer);

	// LayerStack 순회를 위한 Iterator 구현
	[[nodiscard]] inline std::list<std::shared_ptr<Layer>>::iterator begin() { return _layers.begin(); }
	[[nodiscard]] inline std::list<std::shared_ptr<Layer>>::iterator end() { return _layers.end(); }

private:
	std::list<std::shared_ptr<Layer>> _layers;
	std::list<std::shared_ptr<Layer>>::iterator _lastLayerIterator;
};

}