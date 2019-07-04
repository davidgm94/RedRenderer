#include "../common.h"
#include "vulkan.h"
#include "queue.h"
#include "device.h"
#include "buffer.h"
#include "image.h"

#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static inline void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage dstImage, VkBuffer srcBuffer, VkExtent3D imageExtent)
{
	VkCommandBuffer transferCommandBuffer = beginSingleTimeCommands(device, commandPool);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = imageExtent;

	vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(device, commandPool, transferCommandBuffer, queue);
}

VkSampler createTextureSampler(VkDevice device, u32 mipLevels)
{
	VkSamplerCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.mipLodBias = 0.0f;
	createInfo.anisotropyEnable = true;
	createInfo.maxAnisotropy = 16;
	createInfo.compareEnable = false;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = float(mipLevels);
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = false;

	VkSampler sampler = nullptr;
	VKCHECK(vkCreateSampler(device, &createInfo, nullptr, &sampler));

	return sampler;
}

void generateMipmaps(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat imageFormat, int textureWidth, int textureHeight, u32 mipLevels)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

	VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

	int mipWidth = textureWidth;
	int mipHeight = textureHeight;

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcQueueFamilyIndex = 0;
	imageMemoryBarrier.dstQueueFamilyIndex = 0;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	for (u32 i = 0; i < mipLevels - 1; i++)
	{
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = i;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		VkImageBlit imageBlit;
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.mipLevel = i;
		imageBlit.srcSubresource.baseArrayLayer = 0;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.srcOffsets[0] = { 0, 0, 0 };
		imageBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.mipLevel = i + 1;
		imageBlit.dstSubresource.baseArrayLayer = 0;
		imageBlit.dstSubresource.layerCount = 1;
		imageBlit.dstOffsets[0] = { 0, 0, 0 };
		imageBlit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		
		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.srcAccessMask= VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	endSingleTimeCommands(device, commandPool, commandBuffer, queue);
}

Texture loadTexture(const char* texturePath, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkPhysicalDeviceMemoryProperties& memoryProperties, VkSampleCountFlagBits samples, VkFormat textureFormat, VkImageTiling tilingMode, VkImageUsageFlags imageUsage, const QueueInfo& queueInfo)
{
	int textureWidth;
	int textureHeight;
	int textureChannels;

	stbi_uc* texturePixels = stbi_load(texturePath, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
	assert(texturePixels);
	VkDeviceSize textureSize = (u64)textureWidth * (u64)textureHeight * 4;
	VkExtent3D textureExtent = { (u32)textureWidth, (u32)textureHeight, 1 };
	
	Buffer stagingBuffer = createStagingBuffer(device, texturePixels, textureSize, queueInfo, memoryProperties);
	FREE_TEXTURE_PIXELS(texturePixels);

	Texture texture;
	texture.mipLevels = u32(floor(log2(max(textureWidth, textureHeight)))) + 1;
	texture.handle = createImage(device, textureExtent, texture.mipLevels, samples, textureFormat, tilingMode, imageUsage);
	texture.memory = allocateMemoryForImage(device, texture.handle, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryProperties);

	transitionImageLayout(device, commandPool, queue, texture.handle, textureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);
	copyBufferToImage(device, commandPool, queue, texture.handle, stagingBuffer.handle, textureExtent);
	generateMipmaps(physicalDevice, device, commandPool, queue, texture.handle, textureFormat, (i32)textureExtent.width, (i32)textureExtent.height, texture.mipLevels);

	vkDestroyBuffer(device, stagingBuffer.handle, nullptr);
	vkFreeMemory(device, stagingBuffer.memory, nullptr);

	texture.view = createImageView(device, texture.handle, textureFormat, VK_IMAGE_ASPECT_COLOR_BIT, texture.mipLevels);
	texture.sampler = createTextureSampler(device, texture.mipLevels);

	return texture;
}