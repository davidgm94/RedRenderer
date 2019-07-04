#pragma once

VkSwapchainKHR createSwapchain(VkPhysicalDevice physicalDevice, VkExtent2D imageExtent, u32 swapchainImageCount, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkDevice device, VkSurfaceKHR surface);
vector<VkImage> getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain);
vector<VkImageView> createImageViews(VkDevice device, const vector<VkImage>& swapchainImages, VkFormat swapchainImageFormat);
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
	vector<VkDescriptorSet>& descriptorSets, VkImageView& textureImageView, VkSampler& textureSampler, DepthBuffer& depthBuffer, MSAA& msaa);

u32 submitQueue(VkDevice device, VkSwapchainKHR swapchain, VkExtent2D swapchainExtent, u32 currentFrame, VkQueue graphicsQueue, const BufferList& uniformBuffers, const VkCommandBuffer* graphicsCommandBuffers, VkFence* pInflightFence, VkSemaphore* pImageAcquireSemaphore, VkSemaphore* pImageReleaseSemaphore);

VkResult present(VkDevice device, VkSwapchainKHR swapchain, u32 currentFrame, VkQueue graphicsQueue, u32* pImageIndex, VkSemaphore* pImageReleaseSemaphore);
