
#include "../common.h"
#include "vulkan.h"
#include "queue.h"
#include "device.h"

#include "buffer.h"
#include "../glfw.h"
#include "../glm.h"

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = nullptr;
	VKCHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VKCHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	return commandBuffer;
}

void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
	VKCHECK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VKCHECK(vkQueueSubmit(queue, 1, &submitInfo, nullptr));
	VKCHECK(vkQueueWaitIdle(queue));

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

static inline VkBuffer createBuffer(VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, const QueueInfo& queueInfo)
{
	VkBufferCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.size = bufferSize;
	createInfo.usage = usage;
	createInfo.sharingMode = queueInfo.sharingMode;
	createInfo.queueFamilyIndexCount = queueInfo.queueFamilyIndexCount;
	createInfo.pQueueFamilyIndices = queueInfo.pQueueFamilyIndices;

	VkBuffer buffer = nullptr;
	VKCHECK(vkCreateBuffer(device, &createInfo, nullptr, &buffer));

	return buffer;
}

static inline VkDeviceMemory allocateMemoryForBuffer(VkDevice device, VkBuffer buffer, const VkPhysicalDeviceMemoryProperties& memoryProperties, VkMemoryPropertyFlags memoryFlags)
{
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryFlags, memoryProperties);

	VkDeviceMemory bufferMemory = nullptr;
	VKCHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory));

	VKCHECK(vkBindBufferMemory(device, buffer, bufferMemory, 0));

	return bufferMemory;
}

void copyDataToMemory(VkDevice device, VkDeviceMemory memory, u64 dataSize, const void* dataToCopy)
{
	void* data;
	VKCHECK(vkMapMemory(device, memory, 0, dataSize, 0, &data));
	memcpy(data, dataToCopy, dataSize);
	vkUnmapMemory(device, memory);
}

void copyBuffer(VkDevice device, VkCommandPool transferCommandPool, VkQueue transferQueue,
	VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer transferCommandBuffer = beginSingleTimeCommands(device, transferCommandPool);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(device, transferCommandPool, transferCommandBuffer, transferQueue);
}

Buffer createStagingBuffer(VkDevice device, const void* bufferData, u64 bufferSize, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	Buffer stagingBuffer;

	stagingBuffer.handle = createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, queueInfo);
	stagingBuffer.memory = allocateMemoryForBuffer(device, stagingBuffer.handle, memoryProperties, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	copyDataToMemory(device, stagingBuffer.memory, bufferSize, bufferData);

	return stagingBuffer;
}

Buffer createLocalDeviceBuffer(VkDevice device, u64 bufferSize, VkBufferUsageFlags bufferUsage, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	Buffer deviceBuffer = {};

	deviceBuffer.handle = createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage, queueInfo);
	deviceBuffer.memory = allocateMemoryForBuffer(device, deviceBuffer.handle, memoryProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	return deviceBuffer;
}

Buffer bufferDataIntoLocalDevice(VkDevice device, const void* bufferData, u64 bufferSize, VkBufferUsageFlags bufferUsage, VkCommandPool transferCommandPool, VkQueue transferQueue, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	Buffer stagingBuffer = createStagingBuffer(device, bufferData, bufferSize, queueInfo, memoryProperties);
	Buffer deviceBuffer = createLocalDeviceBuffer(device, bufferSize, bufferUsage, queueInfo, memoryProperties);
	
	// Copy data to the buffer
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is slower than using vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges
	// vkFlushMappedMemoryRanges must be called after writing to the mapped memory
	// vkInvalidateMappedMemoryRanges must be called before reading from the mapped memory
	// but actually we're copying to the local device, so it should be fast

	copyDataToMemory(device, stagingBuffer.memory, bufferSize, bufferData);

	copyBuffer(device, transferCommandPool, transferQueue, stagingBuffer.handle, deviceBuffer.handle, bufferSize);

	vkDestroyBuffer(device, stagingBuffer.handle, nullptr);
	vkFreeMemory(device, stagingBuffer.memory, nullptr);

	return deviceBuffer;
}

BufferList createUniformBuffers(VkDevice device, VkDeviceSize swapchainImageCount, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	BufferList uniformBuffers;
	uniformBuffers.handle.resize(swapchainImageCount);
	uniformBuffers.memory.resize(swapchainImageCount);

	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		uniformBuffers.handle[i] = createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, queueInfo);
		uniformBuffers.memory[i] = allocateMemoryForBuffer(device, uniformBuffers.handle[i], memoryProperties, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	return uniformBuffers;
}

void updateUniformBuffer(VkDevice device, const VkExtent2D& swapchainExtent, const BufferList& uniformBuffers, u32 currentImageIndex)
{
	const static double startTime = glfwGetTime();
	const double currentTime = glfwGetTime();
	const float time = (float)(currentTime - startTime);

	UniformBufferObject ubo;
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.f), (float)swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f);
	// Invert to prevent image to be rendered upside down
	ubo.proj[1][1] *= -1.0f;

	copyDataToMemory(device, uniformBuffers.memory[currentImageIndex], sizeof(ubo), &ubo);
}