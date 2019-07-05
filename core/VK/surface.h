#pragma once

struct SurfaceFeatures
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> presentModes;
};

SurfaceFeatures getSurfaceFeatures(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
VkSurfaceFormatKHR pickSurfaceFormat(const vector<VkSurfaceFormatKHR>& surfaceFormats);
VkPresentModeKHR pickPresentMode(const vector<VkPresentModeKHR>& presentModes);
VkExtent2D getSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, VkExtent2D& currentWindowBufferExtent);
u32 getSwapchainImageCount(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);