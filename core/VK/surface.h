#pragma once

struct SurfaceFeatures
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> presentModes;
};

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VkSurfaceKHR createSurface(VkInstance instance, void* platformHandle, void* platformWindow);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
VkSurfaceKHR createSurface(VkInstance instance, ANativeWindow* window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
VkSurfaceKHR createSurface(VkInstance instance, wl_display* display, wl_surface* window);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
VkSurfaceKHR createSurface(VkInstance instance, xcb_connection_t* connection, xcb_window_t window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
VkSurfaceKHR createSurface(VkInstance instance, void* view);
#elif defined(_DIRECT2DISPLAY)
VkSurfaceKHR createSurface(VkInstance instance, uint32_t width, uint32_t height);
#endif

SurfaceFeatures getSurfaceFeatures(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
VkSurfaceFormatKHR pickSurfaceFormat(const vector<VkSurfaceFormatKHR>& surfaceFormats);
VkPresentModeKHR pickPresentMode(const vector<VkPresentModeKHR>& presentModes);
#ifdef GLFW
VkExtent2D getSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, GLFWwindow* window);
#endif
u32 getSwapchainImageCount(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);