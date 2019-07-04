#pragma once

VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device);
VkDescriptorPool createDescriptorPool(VkDevice device, u32 swapchainImageCount);
vector<VkDescriptorSet> createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, u32 swapchainImageCount, VkDescriptorSetLayout descriptorSetLayout, const
	BufferList& uniformBuffers, VkImageView imageView, VkSampler sampler);