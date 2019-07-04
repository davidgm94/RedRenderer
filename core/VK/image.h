#pragma once

VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
VkImage createImage(VkDevice device, VkExtent3D extent, u32 mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
VkDeviceMemory allocateMemoryForImage(VkDevice device, VkImage image, VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties memoryProperties);
VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, u32 mipLevels);
void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, u32 mipLevels);