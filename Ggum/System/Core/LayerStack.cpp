#include "SystemPch.h"
#include "LayerStack.h"


namespace GG{


LayerStack::LayerStack()
{

}

LayerStack::~LayerStack()
{
	_layers.clear();
}

void LayerStack::PushLayer(std::shared_ptr<Layer> layer)
{

}

void LayerStack::PushOverlay(std::shared_ptr<Layer> layer)
{}

void LayerStack::PopLayer(std::shared_ptr<Layer> layer)
{}

void LayerStack::PopOverlay(std::shared_ptr<Layer> layer)
{}

}