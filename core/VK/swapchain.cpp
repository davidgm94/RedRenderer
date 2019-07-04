#include "../common.h"
#include "vulkan.h"
#include "../glfw.h"
#include "../glm.h"
#include "queue.h"
#include "surface.h"
#include "buffer.h"
#include "image.h"
#include "depth.h"
#include "antialiasing.h"

#include "swapchain.h"

#include "pipeline.h"
#include "descriptor.h"


VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice, VkExtent2D imageExtent, u32 swapchainImageCount, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkDevice device, VkSurfaceKHR surface)
{
	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = 2;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = imageExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // change for supporting more than graphics stuff
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // change for supporting more than graphics stuff
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = true;
	createInfo.oldSwapchain = nullptr;

	VkSwapchainKHR swapchain = nullptr;
	VKCHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain));

	return swapchain;
}

vector<VkImage> getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain)
{
	u32 swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr);
	vector<VkImage> swapchainImages(swapchainImageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data());

	return swapchainImages;
}

vector<VkImageView> createImageViews(VkDevice device, const vector<VkImage>& swapchainImages, VkFormat swapchainImageFormat)
{
#define SWAPCHAIN_IMAGE_VIEWS_MIP_LEVELS 1
	const size_t swapchainImageCount = swapchainImages.size();
	vector<VkImageView> imageViews(swapchainImageCount);

	for (size_t i = 0; i < swapchainImageCount; i++)
		imageViews[i] = createImageView(device, swapchainImages[i], swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, SWAPCHAIN_IMAGE_VIEWS_MIP_LEVELS);

#undef SWAPCHAIN_IMAGE_VIEWS_MIP_LEVELS
	return imageViews;
}

void cleanupSwapChain(VkDevice device, VkSwapchainKHR swapchain, const vector<VkFramebuffer>& framebuffers, VkCommandPool commandPool, const vector<VkCommandBuffer>& commandBuffers, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, const vector<VkImageView>& imageViews, const BufferList& uniformBuffers, VkDescriptorPool descriptorPool, DepthBuffer& depthBuffer, MSAA& msaa)
{
	for (size_t i = 0; i < framebuffers.size(); i++)
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);

	vkFreeCommandBuffers(device, commandPool, u32(commandBuffers.size()), commandBuffers.data());

	vkDestroyImage(device, depthBuffer.image, nullptr);
	vkFreeMemory(device, depthBuffer.memory, nullptr);
	vkDestroyImageView(device, depthBuffer.view, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	vkDestroyImage(device, msaa.image, nullptr);
	vkFreeMemory(device, msaa.memory, nullptr);
	vkDestroyImageView(device, msaa.view, nullptr);

	for (size_t i = 0; i < imageViews.size(); i++)
	{
		vkDestroyImageView(device, imageViews[i], nullptr);
		vkDestroyBuffer(device, uniformBuffers.handle[i], nullptr);
		vkFreeMemory(device, uniformBuffers.memory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void recreateSwapchain(VkSwapchainKHR& swapchain, VkPhysicalDevice physicalDevice, VkDevice device,
                       VkSurfaceKHR surface, SurfaceFeatures& surfaceFeatures, GLFWwindow* window,
                       const QueueInfo& queueInfo, vector<VkFramebuffer>& framebuffers,
                       VkDescriptorPool& descriptorPool, vector<VkCommandBuffer>& commandBuffers,
                       VkCommandPool commandPool, VkQueue queue, VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout,
                       VkRenderPass& renderPass, VkDescriptorSetLayout* pDescriptorSetLayout,
                       BufferList& uniformBuffers, const VkPhysicalDeviceMemoryProperties& memoryProperties, const VkPhysicalDeviceProperties& physicalDeviceProperties,
                       vector<VkImage>& swapchainImages, vector<VkImageView>& imageViews, VkShaderModule vertexShader,
                       VkShaderModule fragmentShader, VkBuffer& vertexBuffer, VkBuffer& indexBuffer,
                       const vector<Vertex>& vertices, const vector<u32>& indices,
                       vector<VkDescriptorSet>& descriptorSets, VkImageView& textureImageView, VkSampler& textureSampler, DepthBuffer& depthBuffer, MSAA& msaa)
{
	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	VKCHECK(vkDeviceWaitIdle(device));

	cleanupSwapChain(device, swapchain, framebuffers, commandPool, commandBuffers, graphicsPipeline, pipelineLayout, renderPass, imageViews, uniformBuffers, descriptorPool, depthBuffer, msaa);

	// Create swapchain
	surfaceFeatures = getSurfaceFeatures(physicalDevice, surface);
	VkSurfaceFormatKHR swapchainFormatAndColor = pickSurfaceFormat(surfaceFeatures.formats);
	VkPresentModeKHR presentMode = pickPresentMode(surfaceFeatures.presentModes);
	VkExtent2D swapchainExtent = getSwapchainExtent(surfaceFeatures.capabilities, window);
	u32 swapchainMinImageCount = getSwapchainImageCount(surfaceFeatures.capabilities); // this function may be causing bugs
	swapchain = createSwapchain(physicalDevice, swapchainExtent, swapchainMinImageCount, swapchainFormatAndColor, presentMode, device, surface);
	swapchainImages.clear();
	imageViews.clear();
	swapchainImages = getSwapchainImages(device, swapchain);
	imageViews = createImageViews(device, swapchainImages, swapchainFormatAndColor.format);
	msaa = createMultisamplingBuffer(device, commandPool, queue, physicalDeviceProperties, memoryProperties, swapchainFormatAndColor.format, swapchainExtent);
	renderPass = createRenderPass(device, swapchainFormatAndColor.format, depthBuffer.format, msaa.samples);
	pipelineLayout = createPipelineLayout(device, pDescriptorSetLayout);
	graphicsPipeline = createGraphicsPipeline(device, vertexShader, fragmentShader, swapchainExtent, pipelineLayout, renderPass, msaa.samples);
	depthBuffer = createDepthBuffer(physicalDevice, device, commandPool, queue, swapchainExtent, memoryProperties, msaa.samples);
	framebuffers.clear();
	framebuffers = createFramebuffers(device, imageViews, swapchainExtent, renderPass, depthBuffer.view, msaa.view);
	uniformBuffers.memory.clear();
	uniformBuffers.handle.clear();
	uniformBuffers = createUniformBuffers(device, u32(swapchainImages.size()), queueInfo, memoryProperties);
	descriptorPool = createDescriptorPool(device, (u32)swapchainImages.size());
	descriptorSets = createDescriptorSets(device, descriptorPool, u32(swapchainImages.size()), *pDescriptorSetLayout, uniformBuffers, textureImageView, textureSampler);
	commandBuffers.clear();
	commandBuffers = createCommandBuffers(device, commandPool, (u32)framebuffers.size());

	fillRenderPass(renderPass, swapchainExtent, commandBuffers, framebuffers, vertexBuffer, indexBuffer, graphicsPipeline, pipelineLayout, vertices, indices, descriptorSets);
}

u32 submitQueue(VkDevice device, VkSwapchainKHR swapchain, VkExtent2D swapchainExtent, u32 currentFrame, VkQueue graphicsQueue, const BufferList& uniformBuffers, const VkCommandBuffer* graphicsCommandBuffers, VkFence* pInflightFence, VkSemaphore* pImageAcquireSemaphore, VkSemaphore* pImageReleaseSemaphore)
{
	const u32 fencesUsed = 1;
	VKCHECK(vkWaitForFences(device, fencesUsed, pInflightFence, true, ~0ui64));

	u32 imageIndex;

	VKCHECK(vkAcquireNextImageKHR(device, swapchain, ULLONG_MAX, *pImageAcquireSemaphore, nullptr, &imageIndex));

	updateUniformBuffer(device, swapchainExtent, uniformBuffers, imageIndex);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = pImageAcquireSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &graphicsCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = pImageReleaseSemaphore;

	VKCHECK(vkResetFences(device, 1, pInflightFence));
	VKCHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, *pInflightFence));

	return imageIndex;
}

VkResult present(VkDevice device, VkSwapchainKHR swapchain, u32 currentFrame, VkQueue graphicsQueue, u32* pImageIndex, VkSemaphore* pImageReleaseSemaphore)
{
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = pImageReleaseSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = pImageIndex;
	presentInfo.pResults = nullptr;

	return vkQueuePresentKHR(graphicsQueue, &presentInfo);
}