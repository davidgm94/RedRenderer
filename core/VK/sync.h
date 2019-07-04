#pragma once

#include "common.h"
#include "vulkan.h"

#define MAX_FRAMES_IN_FLIGHT 2

struct FrameSyncStruct
{
	array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAcquireSemaphores;
	array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageReleaseSemaphores;
	array<VkFence, MAX_FRAMES_IN_FLIGHT> inflightFences;
	u32 maxFramesInFlight;
	u32 currentFrame;
};

inline FrameSyncStruct createFrameSyncStruct(VkDevice device, const u32 maxFramesInFlight)
{
	FrameSyncStruct fss;

	VkSemaphoreCreateInfo semaphoreCreateInfo;
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	semaphoreCreateInfo.flags = 0;

	VkFenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = nullptr;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	fss.maxFramesInFlight = maxFramesInFlight;
	fss.currentFrame = 0;
	for (size_t i = 0; i < maxFramesInFlight; i++)
	{
		VKCHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &fss.imageAcquireSemaphores[i]));
		VKCHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &fss.imageReleaseSemaphores[i]));
		VKCHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fss.inflightFences[i]));
	}

	return fss;
}

inline void updateCurrentFrame(FrameSyncStruct& frameSync)
{
	frameSync.currentFrame = (frameSync.currentFrame + 1) % frameSync.maxFramesInFlight;
}