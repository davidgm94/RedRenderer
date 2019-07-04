#pragma once

VkPipelineLayout createPipelineLayout(VkDevice device, VkDescriptorSetLayout* pDescriptorSetLayout);
VkRenderPass createRenderPass(VkDevice device, VkFormat swapchainFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount);
VkPipeline createGraphicsPipeline(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader, const VkExtent2D& swapchainExtent, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkSampleCountFlagBits sampleCount);
vector<VkFramebuffer> createFramebuffers(VkDevice device, const vector<VkImageView>& imageViews, const VkExtent2D& swapchainExtent, VkRenderPass renderPass, VkImageView depthImageView, VkImageView colorImageView);
VkCommandPool createCommandPool(VkDevice device, u32 queueFamilyIndex);
vector<VkCommandBuffer> createCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 framebufferCount);
void fillRenderPass(VkRenderPass renderPass, VkExtent2D swapchainExtent, const vector<VkCommandBuffer>& commandBuffers, const vector<VkFramebuffer>& framebuffers, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, const vector<Vertex>& vertices, const vector<u32>& indices, const vector<VkDescriptorSet>& descriptorSets);