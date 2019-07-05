#pragma once

#include "../common.h"
#include "vulkan.h"

#include "queue.h"

u32 getQueueFamilyIndex(VkQueueFlags queueFlags, const vector<VkQueueFamilyProperties>& queueFamilyProperties)
{
	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags & VK_QUEUE_COMPUTE_BIT)
		for (u32 i = 0; i < queueFamilyProperties.size(); i++)
			if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				return i;

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (queueFlags & VK_QUEUE_TRANSFER_BIT)
		for (u32 i = 0; i < queueFamilyProperties.size(); i++)
			if ((queueFamilyProperties[i].queueFlags & queueFlags) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				return i;

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (u32 i = 0; i < queueFamilyProperties.size(); i++)
		if (queueFamilyProperties[i].queueFlags & queueFlags)
			return i;

	assert(!"Couldn't find a matching queue");
	return 0;
}

vector<VkDeviceQueueCreateInfo> setupQueueCreation(const vector<VkQueueFamilyProperties>& queueFamilyProperties, QueueFamilyIndices& queueFamilyIndices, VkQueueFlags requestedQueueTypes)
{
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	const float defaultQueuePriority(0.0f);

	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, queueFamilyProperties);
		VkDeviceQueueCreateInfo queueInfo;
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext = nullptr;
		queueInfo.flags = 0;
		queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.emplace_back(queueInfo);
	}
	else
	{
		queueFamilyIndices.graphics = VK_NULL_HANDLE;
	}

	// Compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
	{
		queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, queueFamilyProperties);
		if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
		{
			VkDeviceQueueCreateInfo	queueInfo;
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.pNext = nullptr;
			queueInfo.flags = 0;
			queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.emplace_back(queueInfo);
		}
	}
	else
	{
		queueFamilyIndices.compute = queueFamilyIndices.graphics;
	}

	// Transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
	{
		queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, queueFamilyProperties);
		if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
		{
			VkDeviceQueueCreateInfo	queueInfo;
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.pNext = nullptr;
			queueInfo.flags = 0;
			queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.emplace_back(queueInfo);
		}
	}
	else
	{
		queueFamilyIndices.transfer = queueFamilyIndices.graphics;
	}

	return queueCreateInfos;
}

QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice physicalDevice)
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	QueueFamilyIndices queueFamilyIndices = {};
	auto garbarge = setupQueueCreation(queueFamilyProperties, queueFamilyIndices, VK_QUEUE_GRAPHICS_BIT);

	return queueFamilyIndices;
}

QueueInfo getQueueInfo(const QueueFamilyIndices& queueFamilyIndices, const u32* queueFamilyIndexArray, u32 queueFamilyIndexArraySize)
{
	const bool32 dedicatedTransferQueue = queueFamilyIndices.graphics != queueFamilyIndices.transfer;
	u32 l_queueFamilyIndices[2] = { queueFamilyIndices.graphics, queueFamilyIndices.transfer };
	QueueInfo queueInfo;

	if (dedicatedTransferQueue)
	{
		queueInfo.pQueueFamilyIndices = queueFamilyIndexArray;
		queueInfo.queueFamilyIndexCount = queueFamilyIndexArraySize;
		queueInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	}
	else
	{
		queueInfo.pQueueFamilyIndices = nullptr;
		queueInfo.queueFamilyIndexCount = 0;
		queueInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	return queueInfo;
}