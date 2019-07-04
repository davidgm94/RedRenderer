#pragma once

struct DepthBuffer
{
	VkFormat format;
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
DepthBuffer createDepthBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkExtent2D& swapchainExtent, const VkPhysicalDeviceMemoryProperties& memoryProperties, VkSampleCountFlagBits samples);