#pragma once

#define VOLK
#ifdef VOLK
#include <volk.h>
#else
#include <vulkan/vulkan.h>
#endif

#ifndef VK_KHR_VALIDATION_LAYER_NAME
#define VK_KHR_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"
#endif

#ifdef _DEBUG
#define VKCHECK(call)								\
{													\
	VkResult result_ = call; 						\
	assert(result_ == VK_SUCCESS);					\
}
#else
#define VKCHECK(call) (call)
#endif