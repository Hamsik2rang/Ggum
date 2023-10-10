#pragma once

#include "vulkan/vulkan.h"

#include "Base.hpp"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"

#include <vector>

namespace GG {

class GraphicsAPI
{
public:
	GraphicsAPI() = delete;
	GraphicsAPI(HWND hWnd, uint32 frameBufferWidth, uint32 frameBufferHeight);
	~GraphicsAPI();
	
	void Init();

	void Draw();
	void WaitDeviceIdle();
	void Release();

	void RenderImGui();

	void Begin();
	void End();

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

	void initImGui();

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
	void createDescriptorPool();

	void recreateSwapChain();
	void cleanupSwapChain();

	void beginCommandBuffer(VkCommandBuffer commandBuffer);
	void endCommandBuffer(VkCommandBuffer commandBuffer);
	void beginRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer);
	void endRenderPass(VkCommandBuffer commandBuffer);
	void bindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);
	void setViewport(VkCommandBuffer commandBuffer, float x, float y, float width, float height);
	void setScissor(VkCommandBuffer commandBuffer, int x = 0, int y = 0);
	void draw(VkCommandBuffer commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance);

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

	VkResult createDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pCallback);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT callback,
		const VkAllocationCallbacks* pAllocator);

	static const uint32				s_maxSubmitIndex = 3;


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
	VkDescriptorPool				_descriptorPool;

	std::vector<VkSemaphore>		_imageAvailableSemaphores;
	std::vector<VkSemaphore>		_renderFinishedSemaphores;
	std::vector<VkFence>			_inFlightFences;
	std::vector<VkFence>			_imagesInFlight;

	uint32							_submitIndex;
	uint32							_imageIndex;
	uint32							_frameBufferWidth;
	uint32							_frameBufferHeight;

	bool							_isBeginCalled[s_maxSubmitIndex];
};

}