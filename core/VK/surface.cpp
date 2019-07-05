//#include "common.h"
//#include "surface.h"
#include "../common.h"
#include "vulkan.h"
#include "surface.h"

SurfaceFeatures getSurfaceFeatures(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	SurfaceFeatures surfaceFeatures;
	VKCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceFeatures.capabilities));

	u32 surfaceFormatCount = 0;
	VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
	assert(surfaceFormatCount > 0);
	surfaceFeatures.formats.resize(surfaceFormatCount);
	VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFeatures.formats.data()));
	assert(surfaceFeatures.formats.size() > 0);

	u32 presentModeCount = 0;
	VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
	assert(presentModeCount > 0);
	surfaceFeatures.presentModes.resize(presentModeCount);
	VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, surfaceFeatures.presentModes.data()));
	assert(surfaceFeatures.presentModes.size() > 0);

	return surfaceFeatures;
}

VkSurfaceFormatKHR pickSurfaceFormat(const vector<VkSurfaceFormatKHR>& surfaceFormats)
{
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		VkSurfaceFormatKHR surfaceFormat;
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		return surfaceFormat;
	}

	for (const VkSurfaceFormatKHR& surfaceFormat : surfaceFormats)
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return surfaceFormat;

	return surfaceFormats[0];
}

VkPresentModeKHR pickPresentMode(const vector<VkPresentModeKHR>& presentModes)
{
	VkPresentModeKHR preferred = VK_PRESENT_MODE_FIFO_KHR;

	for (const VkPresentModeKHR presentMode : presentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return presentMode;
		else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return presentMode;
	}

	return preferred;
}

VkExtent2D getSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, VkExtent2D& currentSurfaceExtent)
{
	if (surfaceCapabilities.currentExtent.width != ~0u)
		return surfaceCapabilities.currentExtent;
	else
	{
		currentSurfaceExtent.width = max(surfaceCapabilities.minImageExtent.width, min(surfaceCapabilities.maxImageExtent.width, currentSurfaceExtent.width));
		currentSurfaceExtent.height = max(surfaceCapabilities.minImageExtent.height, min(surfaceCapabilities.maxImageExtent.height, currentSurfaceExtent.height));
		return currentSurfaceExtent;
	}
}

u32 getSwapchainImageCount(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	u32 swapchainImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && swapchainImageCount > surfaceCapabilities.maxImageCount)
		swapchainImageCount = surfaceCapabilities.maxImageCount;

	return swapchainImageCount;
}