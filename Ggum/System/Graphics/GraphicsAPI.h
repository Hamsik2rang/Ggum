#pragma once

#include "vulkan/vulkan.h"

#include "Base.hpp"

#include <vector>

namespace GG {

class GraphicsAPI
{
public:
	GraphicsAPI() = delete;
	GraphicsAPI(HWND hWnd);
	~GraphicsAPI();
	
	void Init();
	void Draw();
	void WaitDeviceIdle();
	void Release();

	inline VkInstance GetInstance() { return _instance; }
	inline VkDevice GetDevice() { return _device; }
	inline VkSurfaceKHR GetSurface() { return _surface; }

private:

	struct QueueFamilyIndices
	{
		int graphicsFamily = -1;
		int presentFamily = -1;

		bool IsComplete()
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFrameBuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();

	void cleanupSwapChain();

	bool checkValidationLayerSupport();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkShaderModule createShaderModule(uint32* spvCode, size_t size);

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	VkResult createDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pCallback);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT callback,
		const VkAllocationCallbacks* pAllocator);

	HWND							_hWnd;

	VkInstance						_instance;
	VkDevice						_device;
	VkSurfaceKHR					_surface;
	VkPhysicalDevice				_physicalDevice;	
	VkDebugUtilsMessengerEXT		_debugMessenger;
	VkQueue							_graphicsQueue;
	VkQueue							_presentQueue;
	VkSwapchainKHR					_swapChain;
	std::vector<VkImage>			_swapChainImages;
	VkFormat						_swapChainImageFormat;
	VkExtent2D						_swapChainExtent;
	std::vector<VkImageView>		_swapChainImageViews;
	VkRenderPass					_renderPass;
	VkPipelineLayout				_pipelineLayout;
	VkPipeline						_pipeline;
	std::vector<VkFramebuffer>		_swapChainFramebuffers;
	VkCommandPool					_commandPool;
	std::vector<VkCommandBuffer>	_commandBuffers;

	std::vector<VkSemaphore>		_imageAvailableSemaphores;
	std::vector<VkSemaphore>		_renderFinishedSemaphores;
	std::vector<VkFence>			_inFlightFences;
	std::vector<VkFence>			_imagesInFlight;

	size_t							_submitIndex;

	static const size_t				s_maxSubmitIndex = 2;
};

}