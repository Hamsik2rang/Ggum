#pragma once

#include "vulkan/vulkan.h"

namespace GG {

class GraphicsAPI
{
public:
	GraphicsAPI();
	~GraphicsAPI();
	
	void Init();
	void Release();


private:
	VkInstance			_instance;
	VkDevice			_device;
	VkSurfaceKHR		_surface;
	VkPhysicalDevice	_physicalDevice;
};

}