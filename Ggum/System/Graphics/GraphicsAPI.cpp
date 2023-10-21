#include "SystemPch.h"
#include "GraphicsAPI.h"

#include "GraphicsAPI.h"
#include "Core/Log.h"
#include "Utility/Utility.hpp"

#include <vulkan/vulkan_win32.h>
#include <cstring>


namespace GG {

static const std::vector<const char*> validationLayers
{
	"VK_LAYER_KHRONOS_validation",
};

static std::vector<const char*> deviceExtensions
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


static std::vector<const char*> layerExtensions
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};


GraphicsAPI::GraphicsAPI(HWND hWnd, uint32 frameBufferWidth, uint32 frameBufferHeight)
	: _hWnd{ hWnd }
	, _instance{ nullptr }
	, _device{ nullptr }
	, _surface{ nullptr }
	, _physicalDevice{ nullptr }
	, _debugMessenger{ nullptr }
	, _graphicsQueue{ nullptr }
	, _presentQueue{ nullptr }
	, _swapChain{ nullptr }
	, _renderPass{ nullptr }
	, _pipelineLayout{ nullptr }
	, _pipeline{ nullptr }
	, _commandPool{ nullptr }
	, _descriptorPool{ nullptr }
	, _submitIndex{ 0 }
	, _frameBufferWidth{ frameBufferWidth }
	, _frameBufferHeight{ frameBufferHeight }
	, _isBeginCalled{ false, false, false }
	, _isMinimized{ false }
	, _textureBuffer{ nullptr }
	, _textureWidth{ 0 }
	, _textureHeight{ 0 }
{

}

GraphicsAPI::~GraphicsAPI()
{
	WaitDeviceIdle();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Release();
}

void GraphicsAPI::Init()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createDescriptorSetLayout();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createCommandBuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	createDescriptorPool();
	createDescriptorSets();
	createSyncObjects();

	initImGui();
}

void GraphicsAPI::initImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();
	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };


	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init(_hWnd);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = _instance;
	initInfo.PhysicalDevice = _physicalDevice;
	initInfo.Device = _device;
	initInfo.QueueFamily = static_cast<uint32>(findQueueFamilies(_physicalDevice).graphicsFamily);
	initInfo.Queue = _graphicsQueue;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = _descriptorPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 2;
	initInfo.ImageCount = static_cast<uint32>(_swapChainImages.size());
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&initInfo, _renderPass);

	// Upload Fonts
	{
		VkCommandPool commandPool = _commandPool;
		VkCommandBuffer commandBuffer = _commandBuffers[_submitIndex];

		if (vkResetCommandPool(_device, commandPool, 0) != VK_SUCCESS)
		{
			GG_CRITICAL("Fail to reset command pool for ImGui!");
		}
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			GG_CRITICAL("Fail to begin command buffer for ImGui!");
		}

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

		VkSubmitInfo endInfo{};
		endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		endInfo.commandBufferCount = 1;
		endInfo.pCommandBuffers = &commandBuffer;

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			GG_CRITICAL("Fail to end command buffer for ImGui!");
		}
		if (vkQueueSubmit(_graphicsQueue, 1, &endInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			GG_CRITICAL("Fail to submit queue for ImGui!");
		}

		WaitDeviceIdle();
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void GraphicsAPI::Draw()
{
	if (!_isBeginCalled[_imageIndex]) return;
	if (_isMinimized) return;

	beginRenderPass(_commandBuffers[_imageIndex], _swapChainFramebuffers[_imageIndex]);
	bindPipeline(_commandBuffers[_imageIndex], _pipeline);
	bindDescriptorSets(_commandBuffers[_imageIndex]);
	setViewport(_commandBuffers[_imageIndex], 0.0f, 0.0f, static_cast<float>(_swapChainExtent.width), static_cast<float>(_swapChainExtent.height));
	setScissor(_commandBuffers[_imageIndex], 0, 0);
	draw(_commandBuffers[_imageIndex], 6, 1, 0, 0);
	endRenderPass(_commandBuffers[_imageIndex]);
}

void GraphicsAPI::WaitDeviceIdle()
{
	vkDeviceWaitIdle(_device);
}

void GraphicsAPI::Release()
{
	cleanupSwapChain();

	vkDestroySampler(_device, _textureSampler, nullptr);
	vkDestroyImageView(_device, _textureImageView, nullptr);
	vkDestroyImage(_device, _textureImage, nullptr);
	vkFreeMemory(_device, _textureImageMemory, nullptr);
	if (_textureBuffer)
	{
		delete[] _textureBuffer;
		_textureBuffer = nullptr;
	}

	for (size_t i = 0; i < s_maxSubmitIndex; i++)
	{
		vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(_device, _inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(_device, _commandPool, nullptr);

	vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

	vkDestroyDevice(_device, nullptr);

	DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);

	vkDestroySurfaceKHR(_instance, _surface, nullptr);

	vkDestroyInstance(_instance, nullptr);

}

void GraphicsAPI::RenderImGui()
{
	if (!_isBeginCalled[_imageIndex]) return;

	beginRenderPass(_commandBuffers[_imageIndex], _swapChainFramebuffers[_imageIndex]);

	ImDrawData* mainDrawData = ImGui::GetDrawData();

	ImGui_ImplVulkan_RenderDrawData(mainDrawData, _commandBuffers[_imageIndex]);

	endRenderPass(_commandBuffers[_imageIndex]);
}


void GraphicsAPI::Begin()
{
	if (_isMinimized) return;
	vkWaitForFences(_device, 1, &_inFlightFences[_submitIndex], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphores[_submitIndex], VK_NULL_HANDLE, &_imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		GG_CRITICAL("Failed to acquire swap chain image!");
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (_imagesInFlight[_imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(_device, 1, &_imagesInFlight[_imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	_imagesInFlight[_imageIndex] = _inFlightFences[_submitIndex];

	vkResetCommandBuffer(_commandBuffers[_imageIndex], 0);

	beginCommandBuffer(_commandBuffers[_imageIndex]);

	_isBeginCalled[_imageIndex] = true;
}

void GraphicsAPI::End()
{
	if (!_isBeginCalled[_imageIndex]) return;
	if (_isMinimized) return;

	endCommandBuffer(_commandBuffers[_imageIndex]);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_imageAvailableSemaphores[_submitIndex];
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffers[_imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_renderFinishedSemaphores[_submitIndex];

	vkResetFences(_device, 1, &_inFlightFences[_imageIndex]);

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_imageIndex]) != VK_SUCCESS)
	{
		GG_CRITICAL("Fail to submit graphics queue to Render ImGui!");
	}

	VkSemaphore renderCompleteSemaphore = _renderFinishedSemaphores[_submitIndex];
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapChain;
	presentInfo.pImageIndices = &_imageIndex;
	VkResult result = vkQueuePresentKHR(_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// rebuild swapchain.
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		GG_CRITICAL("Fail to present graphics queue!");
	}

	_submitIndex = (_submitIndex + 1) % s_maxSubmitIndex;

	_isBeginCalled[_imageIndex] = false;
}

void GraphicsAPI::SetPixel(uint32 row, uint32 col, uint8* color)
{
	uint32 index = (_textureWidth * row + col) * _textureChannel;
	for (uint32 i = 0; i < 4; i++)
	{
		_textureBuffer[index + i] = color[i];
	}
}

void GraphicsAPI::SetPixel(uint32 row, uint32 col, uint8 r, uint8 g, uint8 b, uint8 a)
{
	uint8 color[]{ r, g, b, a };
	SetPixel(row, col, color);
}

void GraphicsAPI::createInstance()
{
	GG_ASSERT(checkValidationLayerSupport(), "Validation layers requested, but not available.");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "GGUM";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "GG Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = static_cast<uint32>(layerExtensions.size());
	createInfo.ppEnabledExtensionNames = layerExtensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	createInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	populateDebugMessengerCreateInfo(debugCreateInfo);
	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create instance!");
	}
}

void GraphicsAPI::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (createDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger))
	{
		GG_CRITICAL("Failed to set up debug messenger!");
	}
}

void GraphicsAPI::createSurface()
{
	VkWin32SurfaceCreateInfoKHR sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	sInfo.pNext = nullptr;
	sInfo.flags = 0;
	sInfo.hinstance = GetModuleHandle(nullptr);
	sInfo.hwnd = _hWnd;

	if (vkCreateWin32SurfaceKHR(_instance, &sInfo, nullptr, &_surface) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create surface!");
	}
}

void GraphicsAPI::pickPhysicalDevice()
{
	uint32 deviceCount;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		GG_CRITICAL("Failed to find GPUs with vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());
	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE)
	{
		GG_CRITICAL("Failed to find suitable GPU!");
	}

}

void GraphicsAPI::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	createInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create logical device!");
	}

	vkGetDeviceQueue(_device, indices.graphicsFamily, 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, indices.presentFamily, 0, &_presentQueue);
}

void GraphicsAPI::createSwapChain()
{
	SwapChainSupportDetails details = querySwapChainSupport(_physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(details.presentModes);
	VkExtent2D extent = chooseSwapExtent(details.capabilities);

	uint32 imageCount = details.capabilities.minImageCount + 1;	// ???
	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
	{
		imageCount = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = _surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
	uint32 queueFamilyIndices[] = { static_cast<uint32>(indices.graphicsFamily), static_cast<uint32>(indices.presentFamily) };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = details.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
	_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());
	_swapChainImageFormat = surfaceFormat.format;
	_swapChainExtent = extent;
}

void GraphicsAPI::createImageViews()
{
	_swapChainImageViews.resize(_swapChainImages.size());
	for (size_t i = 0, size = _swapChainImageViews.size(); i < size; i++)
	{
		_swapChainImageViews[i] = createImageView(_swapChainImages[i], _swapChainImageFormat);
	}
}

VkImageView GraphicsAPI::createImageView(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void GraphicsAPI::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = _swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
	{
		GG_CRITICAL("failed to create render pass!");
	}
}

void GraphicsAPI::createGraphicsPipeline()
{
	uint32 vs[]
	{
		0x03022307, 0x00000100, 0x0B000D00, 0x32000000, 0x00000000, 0x11000200, 0x01000000, 0x0B000600,
		0x01000000, 0x474C534C, 0x2E737464, 0x2E343530, 0x00000000, 0x0E000300, 0x00000000, 0x01000000,
		0x0F000800, 0x00000000, 0x04000000, 0x6D61696E, 0x00000000, 0x1F000000, 0x23000000, 0x2E000000,
		0x03000300, 0x02000000, 0xC2010000, 0x04000A00, 0x474C5F47, 0x4F4F474C, 0x455F6370, 0x705F7374,
		0x796C655F, 0x6C696E65, 0x5F646972, 0x65637469, 0x76650000, 0x04000800, 0x474C5F47, 0x4F4F474C,
		0x455F696E, 0x636C7564, 0x655F6469, 0x72656374, 0x69766500, 0x05000400, 0x04000000, 0x6D61696E,
		0x00000000, 0x05000500, 0x0C000000, 0x706F7369, 0x74696F6E, 0x73000000, 0x05000300, 0x14000000,
		0x75760000, 0x05000600, 0x1D000000, 0x676C5F50, 0x65725665, 0x72746578, 0x00000000, 0x06000600,
		0x1D000000, 0x00000000, 0x676C5F50, 0x6F736974, 0x696F6E00, 0x06000700, 0x1D000000, 0x01000000,
		0x676C5F50, 0x6F696E74, 0x53697A65, 0x00000000, 0x06000700, 0x1D000000, 0x02000000, 0x676C5F43,
		0x6C697044, 0x69737461, 0x6E636500, 0x06000700, 0x1D000000, 0x03000000, 0x676C5F43, 0x756C6C44,
		0x69737461, 0x6E636500, 0x05000300, 0x1F000000, 0x00000000, 0x05000600, 0x23000000, 0x676C5F56,
		0x65727465, 0x78496E64, 0x65780000, 0x05000400, 0x2E000000, 0x6F757455, 0x56000000, 0x48000500,
		0x1D000000, 0x00000000, 0x0B000000, 0x00000000, 0x48000500, 0x1D000000, 0x01000000, 0x0B000000,
		0x01000000, 0x48000500, 0x1D000000, 0x02000000, 0x0B000000, 0x03000000, 0x48000500, 0x1D000000,
		0x03000000, 0x0B000000, 0x04000000, 0x47000300, 0x1D000000, 0x02000000, 0x47000400, 0x23000000,
		0x0B000000, 0x2A000000, 0x47000400, 0x2E000000, 0x1E000000, 0x00000000, 0x13000200, 0x02000000,
		0x21000300, 0x03000000, 0x02000000, 0x16000300, 0x06000000, 0x20000000, 0x17000400, 0x07000000,
		0x06000000, 0x02000000, 0x15000400, 0x08000000, 0x20000000, 0x00000000, 0x2B000400, 0x08000000,
		0x09000000, 0x06000000, 0x1C000400, 0x0A000000, 0x07000000, 0x09000000, 0x20000400, 0x0B000000,
		0x06000000, 0x0A000000, 0x3B000400, 0x0B000000, 0x0C000000, 0x06000000, 0x2B000400, 0x06000000,
		0x0D000000, 0x0000803F, 0x2C000500, 0x07000000, 0x0E000000, 0x0D000000, 0x0D000000, 0x2B000400,
		0x06000000, 0x0F000000, 0x000080BF, 0x2C000500, 0x07000000, 0x10000000, 0x0F000000, 0x0F000000,
		0x2C000500, 0x07000000, 0x11000000, 0x0D000000, 0x0F000000, 0x2C000500, 0x07000000, 0x12000000,
		0x0F000000, 0x0D000000, 0x2C000900, 0x0A000000, 0x13000000, 0x0E000000, 0x10000000, 0x11000000,
		0x0E000000, 0x12000000, 0x10000000, 0x3B000400, 0x0B000000, 0x14000000, 0x06000000, 0x2B000400,
		0x06000000, 0x15000000, 0x00000000, 0x2C000500, 0x07000000, 0x16000000, 0x15000000, 0x15000000,
		0x2C000500, 0x07000000, 0x17000000, 0x0D000000, 0x15000000, 0x2C000500, 0x07000000, 0x18000000,
		0x15000000, 0x0D000000, 0x2C000900, 0x0A000000, 0x19000000, 0x0E000000, 0x16000000, 0x17000000,
		0x0E000000, 0x18000000, 0x16000000, 0x17000400, 0x1A000000, 0x06000000, 0x04000000, 0x2B000400,
		0x08000000, 0x1B000000, 0x01000000, 0x1C000400, 0x1C000000, 0x06000000, 0x1B000000, 0x1E000600,
		0x1D000000, 0x1A000000, 0x06000000, 0x1C000000, 0x1C000000, 0x20000400, 0x1E000000, 0x03000000,
		0x1D000000, 0x3B000400, 0x1E000000, 0x1F000000, 0x03000000, 0x15000400, 0x20000000, 0x20000000,
		0x01000000, 0x2B000400, 0x20000000, 0x21000000, 0x00000000, 0x20000400, 0x22000000, 0x01000000,
		0x20000000, 0x3B000400, 0x22000000, 0x23000000, 0x01000000, 0x20000400, 0x25000000, 0x06000000,
		0x07000000, 0x20000400, 0x2B000000, 0x03000000, 0x1A000000, 0x20000400, 0x2D000000, 0x03000000,
		0x07000000, 0x3B000400, 0x2D000000, 0x2E000000, 0x03000000, 0x36000500, 0x02000000, 0x04000000,
		0x00000000, 0x03000000, 0xF8000200, 0x05000000, 0x3E000300, 0x0C000000, 0x13000000, 0x3E000300,
		0x14000000, 0x19000000, 0x3D000400, 0x20000000, 0x24000000, 0x23000000, 0x41000500, 0x25000000,
		0x26000000, 0x0C000000, 0x24000000, 0x3D000400, 0x07000000, 0x27000000, 0x26000000, 0x51000500,
		0x06000000, 0x28000000, 0x27000000, 0x00000000, 0x51000500, 0x06000000, 0x29000000, 0x27000000,
		0x01000000, 0x50000700, 0x1A000000, 0x2A000000, 0x28000000, 0x29000000, 0x15000000, 0x0D000000,
		0x41000500, 0x2B000000, 0x2C000000, 0x1F000000, 0x21000000, 0x3E000300, 0x2C000000, 0x2A000000,
		0x3D000400, 0x20000000, 0x2F000000, 0x23000000, 0x41000500, 0x25000000, 0x30000000, 0x14000000,
		0x2F000000, 0x3D000400, 0x07000000, 0x31000000, 0x30000000, 0x3E000300, 0x2E000000, 0x31000000,
		0xFD000100, 0x38000100
	};

	uint32 fs[]
	{
		0x03022307, 0x00000100, 0x0B000D00, 0x14000000, 0x00000000, 0x11000200, 0x01000000, 0x0B000600,
		0x01000000, 0x474C534C, 0x2E737464, 0x2E343530, 0x00000000, 0x0E000300, 0x00000000, 0x01000000,
		0x0F000700, 0x04000000, 0x04000000, 0x6D61696E, 0x00000000, 0x09000000, 0x11000000, 0x10000300,
		0x04000000, 0x07000000, 0x03000300, 0x02000000, 0xC2010000, 0x04000A00, 0x474C5F47, 0x4F4F474C,
		0x455F6370, 0x705F7374, 0x796C655F, 0x6C696E65, 0x5F646972, 0x65637469, 0x76650000, 0x04000800,
		0x474C5F47, 0x4F4F474C, 0x455F696E, 0x636C7564, 0x655F6469, 0x72656374, 0x69766500, 0x05000400,
		0x04000000, 0x6D61696E, 0x00000000, 0x05000400, 0x09000000, 0x636F6C6F, 0x72000000, 0x05000500,
		0x0D000000, 0x74657853, 0x616D706C, 0x65720000, 0x05000300, 0x11000000, 0x75760000, 0x47000400,
		0x09000000, 0x1E000000, 0x00000000, 0x47000400, 0x0D000000, 0x22000000, 0x00000000, 0x47000400,
		0x0D000000, 0x21000000, 0x00000000, 0x47000400, 0x11000000, 0x1E000000, 0x00000000, 0x13000200,
		0x02000000, 0x21000300, 0x03000000, 0x02000000, 0x16000300, 0x06000000, 0x20000000, 0x17000400,
		0x07000000, 0x06000000, 0x04000000, 0x20000400, 0x08000000, 0x03000000, 0x07000000, 0x3B000400,
		0x08000000, 0x09000000, 0x03000000, 0x19000900, 0x0A000000, 0x06000000, 0x01000000, 0x00000000,
		0x00000000, 0x00000000, 0x01000000, 0x00000000, 0x1B000300, 0x0B000000, 0x0A000000, 0x20000400,
		0x0C000000, 0x00000000, 0x0B000000, 0x3B000400, 0x0C000000, 0x0D000000, 0x00000000, 0x17000400,
		0x0F000000, 0x06000000, 0x02000000, 0x20000400, 0x10000000, 0x01000000, 0x0F000000, 0x3B000400,
		0x10000000, 0x11000000, 0x01000000, 0x36000500, 0x02000000, 0x04000000, 0x00000000, 0x03000000,
		0xF8000200, 0x05000000, 0x3D000400, 0x0B000000, 0x0E000000, 0x0D000000, 0x3D000400, 0x0F000000,
		0x12000000, 0x11000000, 0x57000500, 0x07000000, 0x13000000, 0x0E000000, 0x12000000, 0x3E000300,
		0x09000000, 0x13000000, 0xFD000100, 0x38000100
	};

	for (size_t i = 0; i < sizeof(vs) / sizeof(uint32); i++)
	{
		Utility::swap_endian(vs[i]);
	}

	for (size_t i = 0; i < sizeof(fs) / sizeof(uint32); i++)
	{
		Utility::swap_endian(fs[i]);
	}

	VkShaderModule vertShaderModule = createShaderModule(vs, sizeof(vs));
	VkShaderModule fragShaderModule = createShaderModule(fs, sizeof(fs));

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[]{ vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_swapChainExtent.width;
	viewport.height = (float)_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	std::vector<VkDynamicState> dynamicStates
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout; // Optional

	if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState; // Optional	
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(_device, vertShaderModule, nullptr);
	vkDestroyShaderModule(_device, fragShaderModule, nullptr);

}

void GraphicsAPI::createFrameBuffers()
{
	_swapChainFramebuffers.resize(_swapChainImageViews.size());
	for (size_t i = 0; i < _swapChainImageViews.size(); i++)
	{
		VkImageView attachments[]{
			_swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = _swapChainExtent.width;
		framebufferInfo.height = _swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			GG_CRITICAL("Failed to create framebuffer!");
		}
	}
}

void GraphicsAPI::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create command pool");
	}
}

void GraphicsAPI::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(_device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

uint32 GraphicsAPI::findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

	for (uint32 i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & BIT(i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	GG_CRITICAL("Failed to find suitable memory type!");
	return 0;
}

void GraphicsAPI::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{


	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create texture image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		GG_CRITICAL("failed to allocate image memory!");
	}

	vkBindImageMemory(_device, image, imageMemory, 0);
}

void GraphicsAPI::createTextureImage()
{
	VkDeviceSize size = _textureWidth * _textureHeight * _textureChannel;

	GG_ASSERT(size != 0, "Size should't be zero!");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(_device, stagingBufferMemory, 0, size, 0, &data);
	::memcpy(data, _textureBuffer, static_cast<size_t>(size));
	vkUnmapMemory(_device, stagingBufferMemory);

	createImage(_textureWidth,
		_textureHeight,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_textureImage,
		_textureImageMemory
	);

	transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(stagingBuffer, _textureImage, _textureWidth, _textureHeight);
	transitionImageLayout(_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(_device, stagingBuffer, nullptr);
	vkFreeMemory(_device, stagingBufferMemory, nullptr);
}

void GraphicsAPI::createTextureSampler()
{
	VkSamplerCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
	createInfo.anisotropyEnable = VK_TRUE;
	createInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = 0.0f;

	if (vkCreateSampler(_device, &createInfo, nullptr, &_textureSampler) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create texture sampler!");
	}
}

void GraphicsAPI::updateTextureImage()
{

}

void GraphicsAPI::createTextureImageView()
{
	_textureImageView = createImageView(_textureImage, VK_FORMAT_R8G8B8A8_UNORM);
}

VkCommandBuffer GraphicsAPI::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void GraphicsAPI::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_graphicsQueue);

	vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void GraphicsAPI::copyBufferToImage(VkBuffer buffer, VkImage image, uint32 width, uint32 height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent =
	{
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


	endSingleTimeCommands(commandBuffer);
}

void GraphicsAPI::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		GG_CRITICAL("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(commandBuffer);
}

void GraphicsAPI::createCommandBuffers()
{
	_commandBuffers.resize(_swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32>(_commandBuffers.size());

	if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to allocate command buffers!");
	}
}

void GraphicsAPI::createSyncObjects()
{
	_inFlightFences.resize(s_maxSubmitIndex);
	_imagesInFlight.resize(_swapChainImages.size(), VK_NULL_HANDLE);

	_imageAvailableSemaphores.resize(s_maxSubmitIndex);
	_renderFinishedSemaphores.resize(s_maxSubmitIndex);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < s_maxSubmitIndex; i++)
	{
		if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]))
		{
			GG_CRITICAL("Failed to create semaphore!");
		}
	}
}

void GraphicsAPI::createDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	createInfo.maxSets = 1000 * poolSizes.size();
	createInfo.poolSizeCount = static_cast<uint32>(poolSizes.size());
	createInfo.pPoolSizes = poolSizes.data();
	if (vkCreateDescriptorPool(_device, &createInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
	{
		GG_CRITICAL("Fail to create Descriptor pool!");
	}
}

void GraphicsAPI::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create descriptor set layout!");
	}
}

void GraphicsAPI::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(s_maxSubmitIndex, _descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(s_maxSubmitIndex);
	allocInfo.pSetLayouts = layouts.data();

	_descriptorSets.resize(s_maxSubmitIndex);
	if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < s_maxSubmitIndex; i++)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = _textureImageView;
		imageInfo.sampler = _textureSampler;

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = _descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void GraphicsAPI::recreateSwapChain()
{
	WaitDeviceIdle();

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandBuffers();
}

void GraphicsAPI::cleanupSwapChain()
{
	for (size_t i = 0; i < _swapChainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(_device, _swapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(_device, _commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());

	vkDestroyPipeline(_device, _pipeline, nullptr);
	vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
	vkDestroyRenderPass(_device, _renderPass, nullptr);

	for (size_t i = 0; i < _swapChainImageViews.size(); i++)
	{
		vkDestroyImageView(_device, _swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(_device, _swapChain, nullptr);
}

void GraphicsAPI::beginCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to begin recording command buffer!");
	}
}

void GraphicsAPI::endCommandBuffer(VkCommandBuffer commandBuffer)
{
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		GG_CRITICAL("Fail to end command buffer!");
	}
}

void GraphicsAPI::beginRenderPass(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer)
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _renderPass;
	renderPassInfo.framebuffer = framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = _swapChainExtent;

	VkClearValue clearColor = { {{1.0f, 0.0f, 1.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void GraphicsAPI::endRenderPass(VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
}

void GraphicsAPI::bindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void GraphicsAPI::bindDescriptorSets(VkCommandBuffer commandBuffer)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[_imageIndex], 0, nullptr);
}

void GraphicsAPI::setViewport(VkCommandBuffer commandBuffer, float x, float y, float width, float height)
{
	VkViewport viewport{};
	viewport.x = x;
	viewport.y = y;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void GraphicsAPI::setScissor(VkCommandBuffer commandBuffer, int x, int y)
{
	VkRect2D scissor{};
	scissor.offset = { x, y };
	scissor.extent = _swapChainExtent;

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void GraphicsAPI::draw(VkCommandBuffer commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
	vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

bool GraphicsAPI::checkValidationLayerSupport()
{
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (::strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}

		}
		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

bool GraphicsAPI::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool isExtensionSupported = checkDeviceExtensionSupport(device);
	bool isSwapChainAdequate = false;
	if (isExtensionSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		isSwapChainAdequate = !(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty());
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.IsComplete() && isExtensionSupported && isSwapChainAdequate && supportedFeatures.samplerAnisotropy;
}

GraphicsAPI::QueueFamilyIndices GraphicsAPI::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		VkBool32 isPresentSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &isPresentSupported);
		if (isPresentSupported)
		{
			indices.presentFamily = i;
		}
		i++;

		if (indices.IsComplete())
		{
			break;
		}
	}

	return indices;
}

bool GraphicsAPI::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
		if (requiredExtensions.empty())
		{
			break;
		}
	}

	return requiredExtensions.empty();
}

GraphicsAPI::SwapChainSupportDetails GraphicsAPI::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

	uint32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
	}

	uint32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		GG_WARNING("Validation Layer : {0}", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		GG_TRACE("Validation Layer : {0}", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		GG_CRITICAL("vaildation layer : {0}", pCallbackData->pMessage);
	}
	else
	{
		GG_INFO("Validation Layer : {0}", pCallbackData->pMessage);
	}
	return VK_FALSE;
}

void GraphicsAPI::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& info)
{
	info = {};
	info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	info.pfnUserCallback = debug_callback;
}

VkPresentModeKHR GraphicsAPI::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D GraphicsAPI::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	VkExtent2D extent;
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		extent = capabilities.currentExtent;
	}
	else
	{
		uint32 width, height;
		RECT rect;
		::GetWindowRect(_hWnd, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		VkExtent2D actualExtent =
		{
			static_cast<uint32>(width),
			static_cast<uint32>(height)
		};

		actualExtent.width = Utility::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = Utility::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		extent = actualExtent;
	}

	if (!_textureBuffer || _textureWidth != extent.width || _textureHeight != extent.height)
	{
		_textureWidth = extent.width;
		_textureHeight = extent.height;
		createTextureBuffer(_textureWidth, _textureHeight);
	}

	return extent;
}

VkSurfaceFormatKHR GraphicsAPI::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

void GraphicsAPI::createTextureBuffer(uint32 width, uint32 height)
{
	if (_textureBuffer)
	{
		delete[] _textureBuffer;
		_textureBuffer = nullptr;
	}

	_textureBuffer = new uint8[width * height * _textureChannel]{ 0 };
	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 25; j++)
		{
			SetPixel(i, j, 255, 0, 0, 255);
		}
	}
}

VkShaderModule GraphicsAPI::createShaderModule(uint32* spvCode, size_t size)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = spvCode;

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		GG_CRITICAL("Failed to create shader module!");
	}

	return shaderModule;
}

VkResult GraphicsAPI::createDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void GraphicsAPI::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}
}