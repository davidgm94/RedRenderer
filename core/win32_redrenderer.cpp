#if _WIN64
#include "common.h"
enum class RenderingAPI
{
	VULKAN = 0x00,
	DIRECTX12 = 0x0
};

u32 width = 1024;
u32 height = 576;

#include "win32.h"
#include "glm.h"
#include "model.h"
#include "VK/vulkan.h"
#include "D3D11/d3d11.h"

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

#define APPLICATION_NAME "Red Renderer - A Vulkan 1.1 Renderer"
bool framebufferResized = false;
bool dedicatedTransferQueue = false;

const string rootDirectory = "../../";
const string shaderBytecodePath = "/shaders/bytecode/";
const string modelsDirectory = "core/models/";
const string texturesDirectory = "core/textures/";

const string vertexShaderBytecodeName = "triangle.vert.spv";
const string fragmentShaderBytecodeName = "triangle.frag.spv";

const string modelName = "taylorswift.obj";
const string textureName = "taylorswift.jpeg";

const string vertexShaderFullPath = rootDirectory + shaderBytecodePath + vertexShaderBytecodeName;
const string fragmentShaderFullPath = rootDirectory + shaderBytecodePath + fragmentShaderBytecodeName;
const string modelFullPath = rootDirectory + modelsDirectory + modelName;
const string textureFullPath = rootDirectory + texturesDirectory + textureName;

int WinMain(HINSTANCE currentInstance, HINSTANCE previousInstance, LPSTR, int)
{
	bool32 vulkan = true;
	bool32 d3d11 = true;
	u64 win32_timerFrequency = win32_getTimerFrequency();

#if VOLK
	VKCHECK(volkInitialize());
#endif
	Win32_ApplicationInfo win32vk;
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
	// Isolate platform specific Vulkan code to facilitate future ports
#ifdef _WIN64
	HINSTANCE win32_instance = currentInstance;
	win32vk.window = win32_createWindow(win32_instance, win32_messageCallback, width, height, "Red Renderer: a Vulkan Engine", &win32vk);
	vk.surface = win32_createVulkanSurface(vk.instance, win32_instance, win32vk.window);
#endif
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
	vk.graphicsPipelineLayout = vulkan_createPipelineLayout(vk.device, &vk.descriptorSetLayout);
	vk.depthStencil = vulkan_createDepthStencil(vk.device, vk.physicalDevice, vk.swapchain.extent, vk.deviceDescription.memoryProperties);
	vk.renderPass = vulkan_createRenderPass(vk.device, vk.swapchain.surfaceFormat.format, vk.depthStencil.depthFormat, vk.msaa.samples);

	vk.graphicsPipeline = vulkan_createGraphicsPipeline(vk.device, vk.VS, vk.FS, vk.swapchain.extent, vk.graphicsPipelineLayout, vk.renderPass, vk.msaa.samples);

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
	vulkan_buildCommandBuffers(vk.renderPass, vk.swapchain.extent, vk.drawCommandBuffers, vk.framebuffers, vk.vertexBuffer.handle, vk.indexBuffer.handle, vk.graphicsPipeline, vk.graphicsPipelineLayout, vk.mesh.vertices, vk.mesh.indices, vk.descriptorSets);

	vk.frameSync = vulkan_createSynchronizationResources(vk.device, MAX_FRAMES_IN_FLIGHT);

	// END VULKAN SETUP

	// INIT DIRECTX SETUP

	Win32_ApplicationInfo win32d3d11;
	const char* d3dWindowTitle = "D3D11 Engine";
	win32d3d11.clientArea.right = 1024;
	win32d3d11.clientArea.bottom = 576;

	win32d3d11.window = win32_createWindow(currentInstance, win32_messageCallback, win32d3d11.clientArea.right, win32d3d11.clientArea.bottom, d3dWindowTitle, &win32d3d11);
	D3D11_Renderer renderer = initDirectX11(win32d3d11.window, win32d3d11.clientArea);
	win32d3d11.apiConfig = &renderer;
	RenderedScene scene = initScene();
	win32d3d11.running = true;
	

	// END DIRECTX SETUP

	win32vk.resizing = false;
	
	win32vk.running = true;
	VkResult swapchainUpToDate = VK_SUCCESS;
	u64 startCount = win32_getTimerValue();

	if (d3d11)
	{
		ShowWindow(win32d3d11.window, SW_SHOW);
	}
	if (vulkan)
	{
		ShowWindow(win32vk.window, SW_SHOW);
	}
	do
	{
		float deltaT = win32_deltaT(startCount, win32_timerFrequency);
		if (vulkan)
		{
			SwapchainStatus swapchainStatus = vulkan_updateSwapchain(vk.swapchain, vk.device, vk.physicalDevice, vk.surface);

			if (swapchainStatus == SwapchainStatus::RESIZED)
			{
				vulkan_onWindowResize(&vk);
			}
			else if (swapchainStatus == SwapchainStatus::NOT_READY)
			{
				WIN32_HANDLE_MESSAGES_DEFAULT(win32vk.window);
				continue;
			}
			// GPU fences
			const u32 fencesUsed = 1;
			VKCHECK(vkWaitForFences(vk.device, fencesUsed, vk.frameSync.inflightFences.data(), true, ~0ui64));
			// Send info to local device
			u32 imageIndex;
			VKCHECK(vkAcquireNextImageKHR(vk.device, vk.swapchain.handle, ULLONG_MAX, vk.frameSync.imageAcquireSemaphores[vk.frameSync.currentFrame], nullptr, &imageIndex));
			vulkan_updateUniformBuffer(vk.device, vk.swapchain.extent, vk.uniformBuffers.memory[imageIndex], deltaT);

			// RENDER:
			vulkan_submitQueue(vk.device, vk.graphicsQueue, vk.drawCommandBuffers[imageIndex], vk.frameSync.inflightFences.data(), &vk.frameSync.imageAcquireSemaphores[vk.frameSync.currentFrame], &vk.frameSync.imageReleaseSemaphores[vk.frameSync.currentFrame]);
			VKCHECK(vulkan_present(vk.device, vk.swapchain.handle, vk.frameSync.currentFrame, vk.graphicsQueue, &imageIndex, &vk.frameSync.imageReleaseSemaphores[vk.frameSync.currentFrame]));
			vulkan_updateCurrentFrame(vk.frameSync);

			WIN32_HANDLE_MESSAGES_DEFAULT(win32vk.window);
		}

		if (d3d11)
		{
			update(scene, deltaT);
			render(renderer, scene);

			WIN32_HANDLE_MESSAGES_DEFAULT(win32d3d11.window);
		}		
	}
	while (win32vk.running && win32d3d11.running);

	destroyVulkanApplication(vk);
	shutdownD3D11Renderer(renderer);
}
#endif