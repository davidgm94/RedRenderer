//#include "common.h"
//#include "surface.h"
#include "../common.h"
#include "vulkan.h"
#include "../glfw.h"
#include "surface.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkSurfaceKHR createSurface(VkInstance instance, void* platformHandle, void* platformWindow)
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
VkSurfaceKHR createSurface(VkInstance instance, ANativeWindow* window)
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
VkSurfaceKHR createSurface(VkInstance instance, wl_display* display, wl_surface* window)
#elif defined(VK_USE_PLATFORM_XCB_KHR)
VkSurfaceKHR createSurface(VkInstance instance, xcb_connection_t* connection, xcb_window_t window)
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
VkSurfaceKHR createSurface(VkInstance instance, void* view)
#elif defined(_DIRECT2DISPLAY)
VkSurfaceKHR createSurface(VkInstance instance, uint32_t width, uint32_t height)
#endif
{
	VkSurfaceKHR surface = nullptr;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
	surfaceCreateInfo.hwnd = (HWND)platformWindow;
	VKCHECK(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.window = window;
	VKCHECK(vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, &surface));
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.pView = view;
	VKCHECK(vkCreateIOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, &surface));
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.pView = view;
	VKCHECK(vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, NULL, &surface));
#elif defined(_DIRECT2DISPLAY)
	createDirect2DisplaySurface(width, height);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.display = display;
	surfaceCreateInfo.surface = window;
	VKCHECK(vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.connection = connection;
	surfaceCreateInfo.window = window;
	VKCHECK(vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));
#endif
	return surface;
}

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

#ifdef GLFW
VkExtent2D getSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, GLFWwindow* window)
{
	if (surfaceCapabilities.currentExtent.width != ~0u)
		return surfaceCapabilities.currentExtent;
	else
	{
		int width = 0, height = 0;

		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D extent = { u32(width), u32(height) };

		extent.width = max(surfaceCapabilities.minImageExtent.width, min(surfaceCapabilities.maxImageExtent.width, extent.width));
		extent.height = max(surfaceCapabilities.minImageExtent.height, min(surfaceCapabilities.maxImageExtent.height, extent.height));
		return extent;
	}
}
#endif

u32 getSwapchainImageCount(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	u32 swapchainImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && swapchainImageCount > surfaceCapabilities.maxImageCount)
		swapchainImageCount = surfaceCapabilities.maxImageCount;

	return swapchainImageCount;
}