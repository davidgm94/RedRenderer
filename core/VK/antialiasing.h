#pragma once

struct MSAA
{
	VkSampleCountFlagBits samples;
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};

VkSampleCountFlagBits pickSampleCount(const VkPhysicalDeviceProperties& physicalDeviceProperties);

MSAA createMultisamplingBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkPhysicalDeviceProperties& physicalDeviceProperties, const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties, VkFormat swapchainImageFormat, VkExtent2D swapchainExtent);