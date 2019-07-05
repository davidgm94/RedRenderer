#ifdef __linux__
#include "common.h"
enum class RED_RENDERER_GRAPHICS_API : u32
{
	NONE = 0x0,
	OPENGL = (1 << 1),
	DIRECTX_11 = (1 << 2),
	DIRECTX_12 = (1 << 3),
	VULKAN = (1 << 4),
};
#endif

/*
#ifdef __linux__

enum class RED_RENDERER_GRAPHICS_API : unsigned int
{
	NONE = 0x0,
	OPENGL = (1 << 1),
	DIRECTX_11 = (1 << 2),
	DIRECTX_12 = (1 << 3),
	VULKAN = (1 << 4),
};

#define OPENGL 0
#define DIRECTX11 0
#define DIRECTX12 0
#define VULKAN 1

#if VULKAN

#include "common.h"
#include "glfw.h"
#include "glm.h"
#include "tol.h"
#include "VK/sync.h"
#include "VK/queue.h"
#include "VK/device.h"
#include "VK/surface.h"
#include "VK/shader.h"
#include "VK/pipeline.h"
#include "VK/buffer.h"
#include "VK/descriptor.h"
#include "VK/image.h"
#include "VK/texture.h"
#include "VK/depth.h"
#include "VK/antialiasing.h"
#include "VK/swapchain.h"

VkSurfaceKHR createSurface(VkInstance instance, HINSTANCE platformHandle, HWND platformWindow)
{
	VkSurfaceKHR surface = nullptr;
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = platformHandle;
	surfaceCreateInfo.hwnd = platformWindow;
	VKCHECK(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));

	return surface;
}
#ifdef GLFW
VkExtent2D updateSurfaceExtent(GLFWwindow* window)
{
	int width = 0;
	int height = 0;

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}
	return { u32(width), u32(height) };
}
#endif

#define APPLICATION_NAME "Red Renderer - A Vulkan 1.1 Renderer"
#define WIDTH 1024
#define HEIGHT 576
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

int main(int argc, const char* argv[])
{
#ifdef GLFW
	assert(glfwInit());
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
#ifdef VOLK
	VKCHECK(volkInitialize());
#endif
	VkInstance instance = createInstance();
#ifdef _DEBUG
	VkDebugReportCallbackEXT debugCallback = createDebugCallback(instance,
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
		defaultDebugCallback);
#if _WIN64
	OutputDebugStringA("\n");
#endif
#endif
#ifdef GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, APPLICATION_NAME, nullptr, nullptr);
	glfwSetWindowUserPointer(window, &framebufferResized);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	// need GLFW_EXPOSE_NATIVE_WIN32 for glfwGetWin32Window()
	VkSurfaceKHR surface = createSurface(instance, GetModuleHandle(nullptr), glfwGetWin32Window(window));
#endif
	VkPhysicalDevice physicalDevice = pickPhysicalDevice(instance, surface);
	// WARNING: Possible queue family indices and queue configuration error
	PhysicalDeviceDescription physicalDeviceDescription = getPhysicalDeviceDescription(physicalDevice, surface);
	dedicatedTransferQueue = physicalDeviceDescription.queueFamilyIndices.graphics != physicalDeviceDescription.queueFamilyIndices.transfer;

	VkDevice device = createDevice(physicalDevice, physicalDeviceDescription);
	VkQueue graphicsQueue = nullptr;
	VkQueue transferQueue = nullptr;
	vkGetDeviceQueue(device, physicalDeviceDescription.queueFamilyIndices.graphics, 0, &graphicsQueue);
	if (dedicatedTransferQueue)
		vkGetDeviceQueue(device, physicalDeviceDescription.queueFamilyIndices.transfer, 0, &transferQueue);

	const u32 queueFamilyIndexArray[2] = { physicalDeviceDescription.queueFamilyIndices.graphics, physicalDeviceDescription.queueFamilyIndices.transfer };
	QueueInfo queueInfo = getQueueInfo(physicalDeviceDescription.queueFamilyIndices, queueFamilyIndexArray, ARRAYSIZE(queueFamilyIndexArray));

	SurfaceFeatures surfaceFeatures = getSurfaceFeatures(physicalDevice, surface);
	VkSurfaceFormatKHR swapchainFormatAndColor = pickSurfaceFormat(surfaceFeatures.formats);
	VkPresentModeKHR presentMode = pickPresentMode(surfaceFeatures.presentModes);
#ifdef GLFW
	VkExtent2D currentSurfaceExtent;
	glfwGetFramebufferSize(window, (int*)& currentSurfaceExtent.width, (int*)& currentSurfaceExtent.height);
#endif
	VkExtent2D swapchainExtent = getSwapchainExtent(surfaceFeatures.capabilities, currentSurfaceExtent);
	u32 swapchainMinImageCount = getSwapchainImageCount(surfaceFeatures.capabilities); // this function may be causing bugs
	VkSwapchainKHR swapchain = createSwapchain(physicalDevice, swapchainExtent, swapchainMinImageCount, swapchainFormatAndColor, presentMode, device, surface);

	vector<VkImage> swapchainImages = getSwapchainImages(device, swapchain);
	vector<VkImageView> imageViews = createImageViews(device, swapchainImages, swapchainFormatAndColor.format);

	VkCommandPool graphicsCommandPool = createCommandPool(device, physicalDeviceDescription.queueFamilyIndices.graphics);
	VkCommandPool transferCommandPool = createCommandPool(device, physicalDeviceDescription.queueFamilyIndices.transfer);

	MSAA msaa = createMultisamplingBuffer(device, graphicsCommandPool, graphicsQueue, physicalDeviceDescription.properties, physicalDeviceDescription.memoryProperties, swapchainFormatAndColor.format, swapchainExtent);

	VkShaderModule vertexShader = createShaderModule(device, vertexShaderFullPath.c_str());
	VkShaderModule fragmentShader = createShaderModule(device, fragmentShaderFullPath.c_str());
	VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(device);
	VkPipelineLayout pipelineLayout = createPipelineLayout(device, &descriptorSetLayout);
	VkRenderPass renderPass = createRenderPass(device, swapchainFormatAndColor.format, findDepthFormat(physicalDevice), msaa.samples);


	VkPipeline graphicsPipeline = createGraphicsPipeline(device, vertexShader, fragmentShader, swapchainExtent, pipelineLayout, renderPass, msaa.samples);

	DepthBuffer depthBuffer = createDepthBuffer(physicalDevice, device, graphicsCommandPool, graphicsQueue, swapchainExtent, physicalDeviceDescription.memoryProperties, msaa.samples);
	vector<VkFramebuffer> framebuffers = createFramebuffers(device, imageViews, swapchainExtent, renderPass, depthBuffer.view, msaa.view);
	vector<VkCommandBuffer> graphicsCommandBuffers = createCommandBuffers(device, graphicsCommandPool, (u32)framebuffers.size());

	Texture texture = loadTexture(textureFullPath.c_str(), physicalDevice, device, graphicsCommandPool, graphicsQueue, physicalDeviceDescription.memoryProperties, VK_SAMPLE_COUNT_1_BIT);
	Mesh mesh = loadModel(modelFullPath.c_str());
	Buffer vertexBuffer = bufferDataIntoLocalDevice(device, mesh.vertices.data(), CONTAINER_BYTES(mesh.vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, transferCommandPool, transferQueue, queueInfo, physicalDeviceDescription.memoryProperties);
	Buffer indexBuffer = bufferDataIntoLocalDevice(device, mesh.indices.data(), CONTAINER_BYTES(mesh.indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, transferCommandPool, transferQueue, queueInfo, physicalDeviceDescription.memoryProperties);

	BufferList uniformBuffers = createUniformBuffers(device, swapchainImages.size(), queueInfo, physicalDeviceDescription.memoryProperties);
	VkDescriptorPool descriptorPool = createDescriptorPool(device, (u32)swapchainImages.size());
	vector<VkDescriptorSet> descriptorSets = createDescriptorSets(device, descriptorPool, u32(swapchainImages.size()), descriptorSetLayout, uniformBuffers, texture.view, texture.sampler);
	fillRenderPass(renderPass, swapchainExtent, graphicsCommandBuffers, framebuffers, vertexBuffer.handle, indexBuffer.handle, graphicsPipeline, pipelineLayout, mesh.vertices, mesh.indices, descriptorSets);

	FrameSyncStruct frameSync = createFrameSyncStruct(device, MAX_FRAMES_IN_FLIGHT);

#ifdef GLFW
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
#endif
		u32 imageIndex = submitQueue(device, swapchain, swapchainExtent, frameSync.currentFrame, graphicsQueue, uniformBuffers, graphicsCommandBuffers.data(), frameSync.inflightFences.data(), &frameSync.imageAcquireSemaphores[frameSync.currentFrame], &frameSync.imageReleaseSemaphores[frameSync.currentFrame]);
		VkResult swapchainUpToDate = present(device, swapchain, frameSync.currentFrame, graphicsQueue, &imageIndex, &frameSync.imageReleaseSemaphores[frameSync.currentFrame]);
		if (swapchainUpToDate == VK_ERROR_OUT_OF_DATE_KHR || swapchainUpToDate == VK_SUBOPTIMAL_KHR || framebufferResized)
		{
#ifdef GLFW
			currentSurfaceExtent = updateSurfaceExtent(window);
#endif
			recreateSwapchain(swapchain, physicalDevice, device, surface, surfaceFeatures, currentSurfaceExtent, queueInfo, framebuffers, descriptorPool, graphicsCommandBuffers,
				graphicsCommandPool, graphicsQueue, graphicsPipeline, pipelineLayout, renderPass, &descriptorSetLayout, uniformBuffers, physicalDeviceDescription.memoryProperties, physicalDeviceDescription.properties, swapchainImages, imageViews, vertexShader, fragmentShader, vertexBuffer.handle, indexBuffer.handle, mesh.vertices, mesh.indices, descriptorSets, texture.view, texture.sampler, depthBuffer, msaa);
			framebufferResized = false;
		}
		else
		{
			VKCHECK(swapchainUpToDate);
		}

		updateCurrentFrame(frameSync);
	}

	VKCHECK(vkDeviceWaitIdle(device));

	// Destroy Vulkan

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, frameSync.imageAcquireSemaphores[i], nullptr);
		vkDestroySemaphore(device, frameSync.imageReleaseSemaphores[i], nullptr);
		vkDestroyFence(device, frameSync.inflightFences[i], nullptr);
	}

	vkDestroyBuffer(device, vertexBuffer.handle, nullptr);
	vkFreeMemory(device, vertexBuffer.memory, nullptr);

	vkDestroyBuffer(device, indexBuffer.handle, nullptr);
	vkFreeMemory(device, indexBuffer.memory, nullptr);

	for (size_t i = 0; i < uniformBuffers.memory.size(); i++)
	{
		vkDestroyBuffer(device, uniformBuffers.handle[i], nullptr);
		vkFreeMemory(device, uniformBuffers.memory[i], nullptr);
	}

	vkDestroyImage(device, depthBuffer.image, nullptr);
	vkFreeMemory(device, depthBuffer.memory, nullptr);
	vkDestroyImageView(device, depthBuffer.view, nullptr);

	vkDestroyImage(device, msaa.image, nullptr);
	vkFreeMemory(device, msaa.memory, nullptr);
	vkDestroyImageView(device, msaa.view, nullptr);

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
	vkDestroyCommandPool(device, transferCommandPool, nullptr);

	for (const VkFramebuffer framebuffer : framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);

	vkDestroyImage(device, texture.handle, nullptr);
	vkFreeMemory(device, texture.memory, nullptr);
	vkDestroyImageView(device, texture.view, nullptr);
	vkDestroySampler(device, texture.sampler, nullptr);

	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyShaderModule(device, vertexShader, nullptr);
	vkDestroyShaderModule(device, fragmentShader, nullptr);


	for (const VkImageView imageView : imageViews)
		vkDestroyImageView(device, imageView, nullptr);

	vkDestroySwapchainKHR(device, swapchain, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDestroyDevice(device, nullptr);
#ifdef _DEBUG
	vkDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
#endif
	vkDestroyInstance(instance, nullptr);

	// Destroy window
#ifdef GLFW
	glfwDestroyWindow(window);
#endif
}

#endif
*/