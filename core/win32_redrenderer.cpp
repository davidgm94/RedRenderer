#if _WIN64
#include "common.h"
#define RED_OPENGL 0
#define RED_DIRECTX11 0
#define RED_DIRECTX12 0
#define RED_VULKAN 1

u32 width = 1024;
u32 height = 576;

#if RED_VULKAN
#include "win32.h"
#include "glm.h"
#include "model.h"
#include "VK/vulkan.h"

VkSurfaceKHR win32_createVulkanSurface(VkInstance vulkanInstance, HINSTANCE win32_instance, HWND win32_window)
{
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.hinstance = win32_instance;
	createInfo.hwnd = win32_window;
	VkSurfaceKHR win32_vulkanSurface = nullptr;
	VKCHECK(vkCreateWin32SurfaceKHR(vulkanInstance, &createInfo, nullptr, &win32_vulkanSurface));

	return win32_vulkanSurface;
}

double timestamp()
{
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	return double(counter.QuadPart) / double(freq.QuadPart);
}

#define APPLICATION_NAME "Red Renderer - A Vulkan 1.1 Renderer"
bool framebufferResized = false;
bool dedicatedTransferQueue = false;

const string rootDirectory = "../../";
const string shaderBytecodePath = "/shaders/bytecode/";
const string modelsDirectory = "core/models/";
const string texturesDirectory = "core/textures/";

const string vertexShaderBytecodeName = "triangle.vert.spv";
const string fragmentShaderBytecodeName = "triangle.frag.spv";

const string modelName = "chalet.obj";
const string textureName = "chalet.jpg";

const string vertexShaderFullPath = rootDirectory + shaderBytecodePath + vertexShaderBytecodeName;
const string fragmentShaderFullPath = rootDirectory + shaderBytecodePath + fragmentShaderBytecodeName;
const string modelFullPath = rootDirectory + modelsDirectory + modelName;
const string textureFullPath = rootDirectory + texturesDirectory + textureName;

int WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, LPSTR, int)
{
	u64 win32_timerFrequency = win32_getTimerFrequency();

#if VOLK
	VKCHECK(volkInitialize());
#endif
	Win32_AppInfo win32vk;
	VulkanApplication vk;
	win32vk.api = RED_RENDERER_GRAPHICS_API::VULKAN;
	win32vk.apiConfig = &vk;
	win32vk.window = nullptr;
	win32vk.resizing = false;
	win32vk.running = false;
	vk.instance = vulkan_createInstance();
#if _DEBUG
	VkDebugReportFlagsEXT callbackFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	vk.debugCallback = vulkan_createDebugCallback(vk.instance, callbackFlags, vulkan_defaultDebugCallback);
#endif
	HINSTANCE win32_instance = currentInstance;
	win32vk.window = createWindow(win32_instance, win32_messageCallback, width, height, "Red Renderer: a Vulkan Engine", &win32vk);
	vk.surface = win32_createVulkanSurface(vk.instance, win32_instance, win32vk.window);
	vk.physicalDevice = vulkan_pickPhysicalDevice(vk.instance, vk.surface);
	vk.deviceDescription = vulkan_getPhysicalDeviceDescription(vk.physicalDevice, vk.surface);
	vk.device = vulkan_createDevice(vk.physicalDevice, vk.deviceDescription);
	vk.graphicsQueue = nullptr;
	vkGetDeviceQueue(vk.device, vk.deviceDescription.queueFamilyIndices.graphics, 0, &vk.graphicsQueue);

	{
		// TODO: not used at the moment: implement!
		//VkQueue transferQueue = nullptr;
		//dedicatedTransferQueue = vk.deviceDescription.queueFamilyIndices.graphics != vk.deviceDescription.queueFamilyIndices.transfer;
		//if (dedicatedTransferQueue)
		//	vkGetDeviceQueue(vk.device, vk.deviceDescription.queueFamilyIndices.transfer, 0, &transferQueue);

		//const u32 queueFamilyIndexArray[2] = { vk.deviceDescription.queueFamilyIndices.graphics, vk.deviceDescription.queueFamilyIndices.transfer };
		//vk.queueInfo = vulkan_getQueueInfo(physicalDeviceDescription.queueFamilyIndices, queueFamilyIndexArray, ARRAYSIZE(queueFamilyIndexArray));
	}
	
	vk.swapchain = vulkan_createSwapchain(vk.physicalDevice, vk.device, vk.surface, width, height);
	vk.graphicsCommandPool = vulkan_createCommandPool(vk.device, vk.deviceDescription.queueFamilyIndices.graphics);
	vk.msaa = vulkan_createMultisamplingBuffer(vk.device, vk.graphicsCommandPool, vk.graphicsQueue, vk.deviceDescription.properties, vk.deviceDescription.memoryProperties, vk.swapchain.surfaceFormat.format, vk.swapchain.extent, 1);

	vk.VS = vulkan_createShaderModule(vk.device, vertexShaderFullPath.c_str());
	vk.FS = vulkan_createShaderModule(vk.device, fragmentShaderFullPath.c_str());
	vk.descriptorSetLayout = vulkan_createDescriptorSetLayout(vk.device);
	vk.pipelineLayout = vulkan_createPipelineLayout(vk.device, &vk.descriptorSetLayout);
	vk.depthStencil = vulkan_createDepthStencil(vk.device, vk.physicalDevice, vk.swapchain.extent, vk.deviceDescription.memoryProperties);
	vk.renderPass = vulkan_createRenderPass(vk.device, vk.swapchain.surfaceFormat.format, vk.depthStencil.depthFormat, vk.msaa.samples);

	vk.pipeline = vulkan_createGraphicsPipeline(vk.device, vk.VS, vk.FS, vk.swapchain.extent, vk.pipelineLayout, vk.renderPass, vk.msaa.samples);

	vk.framebuffers = vulkan_createFramebuffers(vk.device, vk.swapchain.imageViews, vk.swapchain.extent, vk.renderPass, vk.depthStencil.imageView, vk.msaa.view);
	vk.drawCommandBuffers = vulkan_createCommandBuffers(vk.device, vk.graphicsCommandPool, (u32)vk.framebuffers.size());

	vk.texture = vulkan_loadTexture(textureFullPath.c_str(), vk.physicalDevice, vk.device, vk.graphicsCommandPool, vk.graphicsQueue, vk.deviceDescription.memoryProperties, VK_SAMPLE_COUNT_1_BIT);
	vk.mesh = loadMesh_fast(modelFullPath.c_str());

	const VulkanQueueInfo onlyOneQueue = { nullptr, 0, VK_SHARING_MODE_EXCLUSIVE };

	vk.vertexBuffer = vulkan_bufferDataIntoLocalDevice(vk.device, vk.mesh.vertices.data(), CONTAINER_BYTES(vk.mesh.vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vk.graphicsCommandPool, vk.graphicsQueue, onlyOneQueue, vk.deviceDescription.memoryProperties);
	vk.indexBuffer = vulkan_bufferDataIntoLocalDevice(vk.device, vk.mesh.indices.data(), CONTAINER_BYTES(vk.mesh.indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vk.graphicsCommandPool, vk.graphicsQueue, onlyOneQueue, vk.deviceDescription.memoryProperties);
	vk.uniformBuffers = vulkan_createUniformBuffers(vk.device, vk.swapchain.images.size(), onlyOneQueue, vk.deviceDescription.memoryProperties);
	vk.descriptorPool = vulkan_createDescriptorPool(vk.device, (u32)vk.swapchain.images.size());
	vk.descriptorSets = vulkan_createDescriptorSets(vk.device, vk.descriptorPool, u32(vk.swapchain.images.size()), vk.descriptorSetLayout, vk.uniformBuffers, vk.texture.view, vk.texture.sampler);
	vulkan_buildCommandBuffers(vk.renderPass, vk.swapchain.extent, vk.drawCommandBuffers, vk.framebuffers, vk.vertexBuffer.handle, vk.indexBuffer.handle, vk.pipeline, vk.pipelineLayout, vk.mesh.vertices, vk.mesh.indices, vk.descriptorSets);

	vk.frameSync = vulkan_createSynchronizationResources(vk.device, MAX_FRAMES_IN_FLIGHT);

	MSG msg = {};
	win32vk.resizing = false;
	ShowWindow(win32vk.window, SW_SHOW);
	win32vk.running = true;
	VkResult swapchainUpToDate = VK_SUCCESS;
	u64 startCount = win32_getTimerValue();

	while (win32vk.running)
	{
		while (PeekMessage(&msg, win32vk.window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		VKCHECK(vkDeviceWaitIdle(vk.device));

		SwapchainStatus swapchainStatus = vulkan_updateSwapchain(&vk.swapchain, vk.device, vk.physicalDevice, vk.surface, win32vk.clientArea.right, win32vk.clientArea.bottom);

		if (swapchainStatus == SwapchainStatus::NOT_READY)
		{
			continue;
		}
		if (swapchainStatus == SwapchainStatus::RESIZED)
		{
			vulkan_onWindowResize(&vk);
		}
		
		float deltaT = win32_getDeltaT_SP(startCount, win32_timerFrequency);
		const u32 fencesUsed = 1;
		VKCHECK(vkWaitForFences(vk.device, fencesUsed, vk.frameSync.inflightFences.data(), true, ~0ui64));

		u32 imageIndex;

		VkResult imageAcquisition = (vkAcquireNextImageKHR(vk.device, vk.swapchain.handle, ULLONG_MAX, vk.frameSync.imageAcquireSemaphores[vk.frameSync.currentFrame], nullptr, &imageIndex));

		vulkan_updateUniformBuffer(vk.device, vk.swapchain.extent, vk.uniformBuffers.memory[imageIndex], deltaT);
		vulkan_submitQueue(vk.device, vk.graphicsQueue, vk.drawCommandBuffers[imageIndex], vk.frameSync.inflightFences.data(), &vk.frameSync.imageAcquireSemaphores[vk.frameSync.currentFrame], &vk.frameSync.imageReleaseSemaphores[vk.frameSync.currentFrame]);
		VkResult swapchainUpToDate = vulkan_present(vk.device, vk.swapchain.handle, vk.frameSync.currentFrame, vk.graphicsQueue, &imageIndex, &vk.frameSync.imageReleaseSemaphores[vk.frameSync.currentFrame]);
		vulkan_updateCurrentFrame(vk.frameSync);
	}

	VKCHECK(vkDeviceWaitIdle(vk.device));

	vkDestroyInstance(vk.instance, nullptr);
}
#endif
#endif