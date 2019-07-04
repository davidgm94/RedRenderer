#pragma once

struct QueueFamilyIndices
{
	u32 graphics;
	u32 compute;
	u32 transfer;
};

struct QueueInfo
{
	const u32* pQueueFamilyIndices;
	u32 queueFamilyIndexCount;
	VkSharingMode sharingMode;
};

u32 getQueueFamilyIndex(VkQueueFlags queueFlags, const vector<VkQueueFamilyProperties>& queueFamilyProperties);
vector<VkDeviceQueueCreateInfo> setupQueueCreation(const vector<VkQueueFamilyProperties>& queueFamilyProperties, QueueFamilyIndices& queueFamilyIndices, VkQueueFlags requestedQueueTypes);
QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice physicalDevice);

QueueInfo getQueueInfo(const QueueFamilyIndices& queueFamilyIndices, const u32* queueFamilyIndexArray, u32 queueFamilyIndexArraySize);