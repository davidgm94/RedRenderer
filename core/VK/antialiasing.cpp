#include "../common.h"
#include "vulkan.h"
#include "image.h"
#include "antialiasing.h"

VkSampleCountFlagBits pickSampleCount(const VkPhysicalDeviceProperties& physicalDeviceProperties) {
	VkSampleCountFlags counts = min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);
#if 0
	{
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	}
#endif

	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

MSAA createMultisamplingBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkPhysicalDeviceProperties& physicalDeviceProperties, const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties, VkFormat swapchainImageFormat, VkExtent2D swapchainExtent)
{
	MSAA msaa;
	msaa.samples = pickSampleCount(physicalDeviceProperties);
	msaa.image = createImage(device, { swapchainExtent.width, swapchainExtent.width, 1 }, 1, msaa.samples, swapchainImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	msaa.memory = allocateMemoryForImage(device, msaa.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	msaa.view = createImageView(device, msaa.image, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	transitionImageLayout(device, commandPool, queue, msaa.image, swapchainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

	return msaa;
}
