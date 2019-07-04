#pragma once

struct PhysicalDeviceDescription
{
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vector<string> extensions;
	vector<VkQueueFamilyProperties> queueFamilyProperties;
	QueueFamilyIndices queueFamilyIndices;
	vector<VkDeviceQueueCreateInfo> deviceQueueConfiguration;
};

#ifdef _DEBUG
VkBool32 defaultDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, u64 object,
                              size_t location, i32 messageCode, const char* pLayerPrefix, const char* pMessage,
                              void* pUserData);
VkDebugReportCallbackEXT createDebugCallback(VkInstance instance, VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callback);
#endif

VkInstance createInstance();

u32 getQueueFamilyIndex(VkQueueFlags queueFlags, const vector<VkQueueFamilyProperties>& queueFamilyProperties);
vector<VkDeviceQueueCreateInfo> setupQueueCreation(const vector<VkQueueFamilyProperties>& queueFamilyProperties, QueueFamilyIndices& queueFamilyIndices, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT);
QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice physicalDevice);
PhysicalDeviceDescription getPhysicalDeviceDescription(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties, const VkPhysicalDeviceMemoryProperties& memoryProperties);

VkDevice createDevice(VkPhysicalDevice physicalDevice, const PhysicalDeviceDescription& physicalDeviceDescription);