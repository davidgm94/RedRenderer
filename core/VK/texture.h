#pragma once

struct Texture
{
	u32 mipLevels;
	VkImage handle;
	VkDeviceMemory memory;
	VkImageView view;
	VkSampler sampler;
};

#define FREE_TEXTURE_PIXELS(texture_pixels) stbi_image_free(texture_pixels)

Texture loadTexture(const char* texturePath, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkPhysicalDeviceMemoryProperties& memoryProperties, VkSampleCountFlagBits samples, VkFormat textureFormat = VK_FORMAT_R8G8B8A8_UNORM, VkImageTiling tilingMode = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, const QueueInfo& queueInfo = { nullptr, 0, VK_SHARING_MODE_EXCLUSIVE });
VkSampler createTextureSampler(VkDevice device, u32 mipLevels);