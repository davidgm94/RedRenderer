struct Buffer
{
	VkBuffer handle;
	VkDeviceMemory memory;
};

struct BufferList
{
	vector<VkBuffer> handle;
	vector<VkDeviceMemory> memory;
};

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);
void copyDataToMemory(VkDevice device, VkDeviceMemory memory, u64 dataSize, const void* dataToCopy);
void copyBuffer(VkDevice device, VkCommandPool transferCommandPool, VkQueue transferQueue,
	VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
Buffer createStagingBuffer(VkDevice device, const void* bufferData, u64 bufferSize, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties);
Buffer createLocalDeviceBuffer(VkDevice device, u64 bufferSize, VkBufferUsageFlags bufferUsage, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties);
Buffer bufferDataIntoLocalDevice(VkDevice device, const void* bufferData, u64 bufferSize, VkBufferUsageFlags bufferUsage, VkCommandPool transferCommandPool, VkQueue transferQueue, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties);
BufferList createUniformBuffers(VkDevice device, VkDeviceSize swapchainImageCount, const QueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties);
void updateUniformBuffer(VkDevice device, const VkExtent2D& swapchainExtent, const BufferList& uniformBuffers, u32 currentImageIndex);