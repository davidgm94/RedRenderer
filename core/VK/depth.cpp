#include "../common.h"
#include "vulkan.h"
#include "image.h"

#include "depth.h"

inline VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
{
	return findSupportedFormat(physicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

DepthBuffer createDepthBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkExtent2D& swapchainExtent, const VkPhysicalDeviceMemoryProperties& memoryProperties, VkSampleCountFlagBits samples)
{
#define DEPTH_BUFFER_MIP_LEVELS 1
	DepthBuffer depthBuffer;
	depthBuffer.format = findDepthFormat(physicalDevice);

	depthBuffer.image = createImage(device, { swapchainExtent.width, swapchainExtent.height, 1 }, DEPTH_BUFFER_MIP_LEVELS, samples, depthBuffer.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthBuffer.memory = allocateMemoryForImage(device, depthBuffer.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryProperties);
	depthBuffer.view = createImageView(device, depthBuffer.image, depthBuffer.format, VK_IMAGE_ASPECT_DEPTH_BIT, DEPTH_BUFFER_MIP_LEVELS);

	transitionImageLayout(device, commandPool, queue, depthBuffer.image, depthBuffer.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, DEPTH_BUFFER_MIP_LEVELS);

#undef DEPTH_BUFFER_MIP_LEVELS
	return depthBuffer;
}