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
{

}

GraphicsAPI::~GraphicsAPI()
{
	WaitDeviceIdle();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	ImGui_ImplVulkanH_DestroyWindow(_instance, _device, &_imguiWindow, nullptr);

	//TODO: Destroy Vulkan Handles...
	Release();
}

void GraphicsAPI::Init()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createDescriptorPool();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandPool();
	createCommandBuffers();
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
	beginRenderPass(_commandBuffers[_imageIndex], _swapChainFramebuffers[_imageIndex]);
	bindPipeline(_commandBuffers[_imageIndex], _pipeline);
	bindPipeline(_commandBuffers[_imageIndex], _pipeline);
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

	for (size_t i = 0; i < s_maxSubmitIndex; i++)
	{
		vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(_device, _inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(_device, _commandPool, nullptr);

	vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

	vkDestroyDevice(_device, nullptr);

	DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);

	vkDestroySurfaceKHR(_instance, _surface, nullptr);

	vkDestroyInstance(_instance, nullptr);

}

void GraphicsAPI::RenderImGui()
{
	beginRenderPass(_commandBuffers[_imageIndex], _swapChainFramebuffers[_imageIndex]);

	ImDrawData* mainDrawData = ImGui::GetDrawData();

	ImGui_ImplVulkan_RenderDrawData(mainDrawData, _commandBuffers[_submitIndex]);

	endRenderPass(_commandBuffers[_imageIndex]);
}


void GraphicsAPI::Begin()
{
	vkWaitForFences(_device, 1, &_inFlightFences[_submitIndex], VK_TRUE, UINT64_MAX);

	VkResult result = vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphores[_submitIndex], VK_NULL_HANDLE, &_imageIndex);

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
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
}

void GraphicsAPI::End()
{
	endCommandBuffer(_commandBuffers[_imageIndex]);

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_imageAvailableSemaphores[_imageIndex];
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffers[_imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_renderFinishedSemaphores[_imageIndex];

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
	presentInfo.pImageIndices = &_submitIndex;
	VkResult result = vkQueuePresentKHR(_presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// rebuild swapchain.
		return;
	}
	if (result != VK_SUCCESS)
	{
		GG_CRITICAL("Fail to present graphics queue!");
	}

	_submitIndex = (_submitIndex + 1) % s_maxSubmitIndex;
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
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = _swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = _swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
		{
			GG_CRITICAL("Failed to create image view!");
		}
	}
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
		0x03022307, 0x00000100, 0x0B000D00, 0x28000000, 0x00000000, 0x11000200, 0x01000000, 0x0B000600,
		0x01000000, 0x474C534C, 0x2E737464, 0x2E343530, 0x00000000, 0x0E000300, 0x00000000, 0x01000000,
		0x0F000700, 0x00000000, 0x04000000, 0x6D61696E, 0x00000000, 0x19000000, 0x1D000000, 0x03000300,
		0x02000000, 0xC2010000, 0x04000A00, 0x474C5F47, 0x4F4F474C, 0x455F6370, 0x705F7374, 0x796C655F,
		0x6C696E65, 0x5F646972, 0x65637469, 0x76650000, 0x04000800, 0x474C5F47, 0x4F4F474C, 0x455F696E,
		0x636C7564, 0x655F6469, 0x72656374, 0x69766500, 0x05000400, 0x04000000, 0x6D61696E, 0x00000000,
		0x05000500, 0x0C000000, 0x706F7369, 0x74696F6E, 0x73000000, 0x05000600, 0x17000000, 0x676C5F50,
		0x65725665, 0x72746578, 0x00000000, 0x06000600, 0x17000000, 0x00000000, 0x676C5F50, 0x6F736974,
		0x696F6E00, 0x06000700, 0x17000000, 0x01000000, 0x676C5F50, 0x6F696E74, 0x53697A65, 0x00000000,
		0x06000700, 0x17000000, 0x02000000, 0x676C5F43, 0x6C697044, 0x69737461, 0x6E636500, 0x06000700,
		0x17000000, 0x03000000, 0x676C5F43, 0x756C6C44, 0x69737461, 0x6E636500, 0x05000300, 0x19000000,
		0x00000000, 0x05000600, 0x1D000000, 0x676C5F56, 0x65727465, 0x78496E64, 0x65780000, 0x48000500,
		0x17000000, 0x00000000, 0x0B000000, 0x00000000, 0x48000500, 0x17000000, 0x01000000, 0x0B000000,
		0x01000000, 0x48000500, 0x17000000, 0x02000000, 0x0B000000, 0x03000000, 0x48000500, 0x17000000,
		0x03000000, 0x0B000000, 0x04000000, 0x47000300, 0x17000000, 0x02000000, 0x47000400, 0x1D000000,
		0x0B000000, 0x2A000000, 0x13000200, 0x02000000, 0x21000300, 0x03000000, 0x02000000, 0x16000300,
		0x06000000, 0x20000000, 0x17000400, 0x07000000, 0x06000000, 0x02000000, 0x15000400, 0x08000000,
		0x20000000, 0x00000000, 0x2B000400, 0x08000000, 0x09000000, 0x06000000, 0x1C000400, 0x0A000000,
		0x07000000, 0x09000000, 0x20000400, 0x0B000000, 0x06000000, 0x0A000000, 0x3B000400, 0x0B000000,
		0x0C000000, 0x06000000, 0x2B000400, 0x06000000, 0x0D000000, 0x0000803F, 0x2C000500, 0x07000000,
		0x0E000000, 0x0D000000, 0x0D000000, 0x2B000400, 0x06000000, 0x0F000000, 0x000080BF, 0x2C000500,
		0x07000000, 0x10000000, 0x0F000000, 0x0F000000, 0x2C000500, 0x07000000, 0x11000000, 0x0D000000,
		0x0F000000, 0x2C000500, 0x07000000, 0x12000000, 0x0F000000, 0x0D000000, 0x2C000900, 0x0A000000,
		0x13000000, 0x0E000000, 0x10000000, 0x11000000, 0x0E000000, 0x12000000, 0x10000000, 0x17000400,
		0x14000000, 0x06000000, 0x04000000, 0x2B000400, 0x08000000, 0x15000000, 0x01000000, 0x1C000400,
		0x16000000, 0x06000000, 0x15000000, 0x1E000600, 0x17000000, 0x14000000, 0x06000000, 0x16000000,
		0x16000000, 0x20000400, 0x18000000, 0x03000000, 0x17000000, 0x3B000400, 0x18000000, 0x19000000,
		0x03000000, 0x15000400, 0x1A000000, 0x20000000, 0x01000000, 0x2B000400, 0x1A000000, 0x1B000000,
		0x00000000, 0x20000400, 0x1C000000, 0x01000000, 0x1A000000, 0x3B000400, 0x1C000000, 0x1D000000,
		0x01000000, 0x20000400, 0x1F000000, 0x06000000, 0x07000000, 0x2B000400, 0x06000000, 0x22000000,
		0x00000000, 0x20000400, 0x26000000, 0x03000000, 0x14000000, 0x36000500, 0x02000000, 0x04000000,
		0x00000000, 0x03000000, 0xF8000200, 0x05000000, 0x3E000300, 0x0C000000, 0x13000000, 0x3D000400,
		0x1A000000, 0x1E000000, 0x1D000000, 0x41000500, 0x1F000000, 0x20000000, 0x0C000000, 0x1E000000,
		0x3D000400, 0x07000000, 0x21000000, 0x20000000, 0x51000500, 0x06000000, 0x23000000, 0x21000000,
		0x00000000, 0x51000500, 0x06000000, 0x24000000, 0x21000000, 0x01000000, 0x50000700, 0x14000000,
		0x25000000, 0x23000000, 0x24000000, 0x22000000, 0x0D000000, 0x41000500, 0x26000000, 0x27000000,
		0x19000000, 0x1B000000, 0x3E000300, 0x27000000, 0x25000000, 0xFD000100, 0x38000100
	};

	uint32 fs[]
	{
		0x03022307, 0x00000100, 0x0B000D00, 0x0D000000, 0x00000000, 0x11000200, 0x01000000, 0x0B000600,
		0x01000000, 0x474C534C, 0x2E737464, 0x2E343530, 0x00000000, 0x0E000300, 0x00000000, 0x01000000,
		0x0F000600, 0x04000000, 0x04000000, 0x6D61696E, 0x00000000, 0x09000000, 0x10000300, 0x04000000,
		0x07000000, 0x03000300, 0x02000000, 0xC2010000, 0x04000A00, 0x474C5F47, 0x4F4F474C, 0x455F6370,
		0x705F7374, 0x796C655F, 0x6C696E65, 0x5F646972, 0x65637469, 0x76650000, 0x04000800, 0x474C5F47,
		0x4F4F474C, 0x455F696E, 0x636C7564, 0x655F6469, 0x72656374, 0x69766500, 0x05000400, 0x04000000,
		0x6D61696E, 0x00000000, 0x05000400, 0x09000000, 0x636F6C6F, 0x72000000, 0x47000400, 0x09000000,
		0x1E000000, 0x00000000, 0x13000200, 0x02000000, 0x21000300, 0x03000000, 0x02000000, 0x16000300,
		0x06000000, 0x20000000, 0x17000400, 0x07000000, 0x06000000, 0x04000000, 0x20000400, 0x08000000,
		0x03000000, 0x07000000, 0x3B000400, 0x08000000, 0x09000000, 0x03000000, 0x2B000400, 0x06000000,
		0x0A000000, 0x0000003F, 0x2B000400, 0x06000000, 0x0B000000, 0x0000803F, 0x2C000700, 0x07000000,
		0x0C000000, 0x0A000000, 0x0A000000, 0x0A000000, 0x0B000000, 0x36000500, 0x02000000, 0x04000000,
		0x00000000, 0x03000000, 0xF8000200, 0x05000000, 0x3E000300, 0x09000000, 0x0C000000, 0xFD000100,
		0x38000100
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
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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
	_imageAvailableSemaphores.resize(s_maxSubmitIndex);
	_renderFinishedSemaphores.resize(s_maxSubmitIndex);
	_inFlightFences.resize(s_maxSubmitIndex);
	_imagesInFlight.resize(_swapChainImages.size(), VK_NULL_HANDLE);

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

	return indices.IsComplete() && isExtensionSupported && isSwapChainAdequate;
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
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		GG_INFO("Validation Layer : {0}", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
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
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		RECT rect;
		::GetWindowRect(_hWnd, &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = Utility::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = Utility::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
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