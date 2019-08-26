#pragma once

#include "../common.h"

#define VOLK 1
#if VOLK
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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../glm.h"

#define MAX_FRAMES_IN_FLIGHT 2

struct VulkanFrameSynchronization
{
	array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAcquireSemaphores;
	array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageReleaseSemaphores;
	array<VkFence, MAX_FRAMES_IN_FLIGHT> inflightFences;
	u32 maxFramesInFlight;
	u32 currentFrame;
};

struct VulkanBuffer
{
	VkBuffer handle;
	VkDeviceMemory memory;
};

struct VulkanBufferList
{
	vector<VkBuffer> handle;
	vector<VkDeviceMemory> memory;
};

struct VulkanTexture
{
	u32 mipLevels;
	VkImage handle;
	VkDeviceMemory memory;
	VkImageView view;
	VkSampler sampler;
};

struct VulkanMSAA
{
	VkSampleCountFlagBits samples;
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};

struct VulkanSwapchain
{
	VkSwapchainKHR handle;
	VkExtent2D extent;
	VkPresentModeKHR presentMode;
	vector<VkImage> images;
	vector<VkImageView> imageViews;
	VkSurfaceFormatKHR surfaceFormat;
};

enum class SwapchainStatus : u32
{
	READY = 0x00,
	RESIZED = 0x01,
	NOT_READY = 0x02,
};

struct VulkanDepthStencil
{
	VkImage image;
	VkDeviceMemory memory;
	VkImageView imageView;
	VkFormat depthFormat;
};

struct VulkanQueueFamilyIndices
{
	u32 graphics;
	u32 compute;
	u32 transfer;
};

struct VulkanQueueInfo
{
	const u32* pQueueFamilyIndices;
	u32 queueFamilyIndexCount;
	VkSharingMode sharingMode;
};

struct VulkanPhysicalDeviceDescription
{
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vector<string> extensions;
	vector<VkQueueFamilyProperties> queueFamilyProperties;
	VulkanQueueFamilyIndices queueFamilyIndices;
	vector<VkDeviceQueueCreateInfo> deviceQueueConfiguration;
};

struct VulkanApplication
{
	VkInstance instance;
	VkDebugReportCallbackEXT debugCallback;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	VkDevice device;
	VulkanPhysicalDeviceDescription deviceDescription;
	VkCommandPool graphicsCommandPool;
	VulkanSwapchain swapchain;
	vector<VkCommandBuffer> drawCommandBuffers;
	vector<VkFence> waitFences;
	VulkanDepthStencil depthStencil;
	VkRenderPass renderPass;
	VkPipelineCache pipelineCache;
	vector<VkFramebuffer> framebuffers;
	VkQueue graphicsQueue;
	VulkanMSAA msaa;
	VkShaderModule FS;
	VkShaderModule VS;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout graphicsPipelineLayout;
	VkPipeline graphicsPipeline;
	VulkanTexture texture;
	Mesh mesh;
	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;
	VulkanBufferList uniformBuffers;
	VkDescriptorPool descriptorPool;
	vector<VkDescriptorSet> descriptorSets;
	VulkanFrameSynchronization frameSync;
};

VkInstance vulkan_createInstance()
{
	u32 layerCount = 0;
	VkResult layerPropertiesEnumeration1 = vkEnumerateInstanceLayerProperties(&layerCount, 0);
	VKCHECK(layerPropertiesEnumeration1);
	assert(layerCount > 0);

	vector<VkLayerProperties> layerProperties(layerCount);
	VkResult layerPropertiesEnumeration2 = vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
	VKCHECK(layerPropertiesEnumeration2);

	u32 extensionCount;
	VkResult extensionEnumeration1 = vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);
	VKCHECK(extensionEnumeration1);
	assert(extensionCount > 0);

	vector<VkExtensionProperties> extensionProperties(extensionCount);
	VkResult extensionEnumeration2 = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
	VKCHECK(extensionEnumeration2);
#if !defined(__ANDROID__)
	array<const char*> layers =
	{
#ifdef _DEBUG
		VK_KHR_VALIDATION_LAYER_NAME
#endif
	};
#else
#error "Platform not supported"
#endif

	vector<const char*> extensions =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined _DEBUG
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#if defined(_WIN64)
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#elif defined(_DIRECT2DISPLAY)
	VK_KHR_DISPLAY_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	VK_MVK_IOS_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#endif
	};

	VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Red Renderer sample";
	applicationInfo.applicationVersion = 1;
	applicationInfo.pEngineName = "Red Renderer";
	applicationInfo.engineVersion = 1;
	applicationInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.enabledLayerCount = (u32)layers.size();
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.enabledExtensionCount = (u32)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkInstance instance = nullptr;
	VKCHECK(vkCreateInstance(&createInfo, nullptr, &instance));
#if VOLK
	volkLoadInstance(instance);
#else
#error "Running Vulkan without Volk is not supported"
#endif

	return instance;
}

#ifdef _DEBUG
VkBool32 vulkan_defaultDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, u64 object,
	size_t location, i32 messageCode, const char* pLayerPrefix, const char* pMessage,
	void* pUserData)
{
	// Select prefix depending on flags passed to the callback
			// Note that multiple flags may be set for a single validation message
	char prefix[20];

	// Error that may result in undefined behaviour
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		strcpy(prefix, "ERROR:");
	};
	// Warnings may hint at unexpected / non-spec API usage
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		strcpy(prefix, "WARNING:");
	};
	// May indicate sub-optimal usage of the API
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		strcpy(prefix, "PERFORMANCE:");
	};
	// Informal messages that may become handy during debugging
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		strcpy(prefix, "INFO:");
	}
	// Diagnostic info from the Vulkan loader and layers
	// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		strcpy(prefix, "DEBUG:");
	}

	// Display message to default output (console/logcat)
	char debugMessage[4096];
	sprintf(debugMessage, "%s [%s] Code %d: %s", prefix, pLayerPrefix, messageCode, pMessage);

#if defined(__ANDROID__)
	// This may need some attention
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		LOGE("%s", debugMessage.str().c_str());
	}
	else {
		LOGD("%s", debugMessage.str().c_str());
	}
#else
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		fprintf(stderr, "%s\n", debugMessage);
	}
	else
	{
		printf("%s\n", debugMessage);
	}
#ifdef _WIN64
	OutputDebugStringA(debugMessage);
	OutputDebugStringA("\n");
#endif
#endif

	fflush(stdout);
	// The return value of this callback controls wether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}

VkDebugReportCallbackEXT vulkan_createDebugCallback(VkInstance instance, VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callback)
{
	VkDebugReportCallbackCreateInfoEXT createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.pNext = nullptr;
	createInfo.flags = flags;
	createInfo.pfnCallback = callback;
	createInfo.pUserData = nullptr;

	VkDebugReportCallbackEXT callbackStruct = nullptr;
	VKCHECK(vkCreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callbackStruct));
	return callbackStruct;
}
#endif


static const char* vulkan_physicalDeviceTypeString(VkPhysicalDeviceType type)
{
	switch (type)
	{
#define _LOCAL_TOSTRING(r) case VK_PHYSICAL_DEVICE_TYPE_ ##r: return #r
		_LOCAL_TOSTRING(OTHER);
		_LOCAL_TOSTRING(INTEGRATED_GPU);
		_LOCAL_TOSTRING(DISCRETE_GPU);
		_LOCAL_TOSTRING(VIRTUAL_GPU);
#undef _LOCAL_TOSTRING
		default: return "UNKNOWN_DEVICE";
	}
}

u32 vulkan_getQueueFamilyIndex(VkQueueFlags queueFlags, const vector<VkQueueFamilyProperties>& queueFamilyProperties)
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

vector<VkDeviceQueueCreateInfo> vulkan_setupQueueCreation(const vector<VkQueueFamilyProperties>& queueFamilyProperties, VulkanQueueFamilyIndices& queueFamilyIndices, VkQueueFlags requestedQueueTypes)
{
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	const float defaultQueuePriority(0.0f);

	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		queueFamilyIndices.graphics = vulkan_getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, queueFamilyProperties);
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
		queueFamilyIndices.compute = vulkan_getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT, queueFamilyProperties);
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
		queueFamilyIndices.transfer = vulkan_getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, queueFamilyProperties);
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

VulkanQueueFamilyIndices vulkan_getQueueFamilyIndices(VkPhysicalDevice physicalDevice)
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	VulkanQueueFamilyIndices queueFamilyIndices = {};
	auto garbage = vulkan_setupQueueCreation(queueFamilyProperties, queueFamilyIndices, VK_QUEUE_GRAPHICS_BIT);

	return queueFamilyIndices;
}

VulkanQueueInfo vulkan_getQueueInfo(const VulkanQueueFamilyIndices& queueFamilyIndices, const u32* queueFamilyIndexArray, u32 queueFamilyIndexArraySize)
{
	const bool32 dedicatedTransferQueue = queueFamilyIndices.graphics != queueFamilyIndices.transfer;
	u32 l_queueFamilyIndices[2] = { queueFamilyIndices.graphics, queueFamilyIndices.transfer };
	VulkanQueueInfo queueInfo;

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

VkPhysicalDevice vulkan_pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
	u32 physicalDeviceCount = 0;
	VKCHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	assert(physicalDeviceCount > 0);
	vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	VKCHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

	printf("Listing available GPUs\n----------------------\n");
	u16 GPUIndex = 0;

	vector<VkPhysicalDeviceProperties> deviceProperties(physicalDeviceCount);
	vector<VkPhysicalDeviceFeatures> supportedFeatures(physicalDeviceCount);
	for (const VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures[GPUIndex]);
		vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties[GPUIndex]);
		printf("Device %u: %s\n", GPUIndex, deviceProperties[GPUIndex].deviceName);
		printf("\tType: %s\n", vulkan_physicalDeviceTypeString(deviceProperties[GPUIndex].deviceType));
		printf("\tAPI: %u.%u.%u\n",
			VK_VERSION_MAJOR(deviceProperties[GPUIndex].apiVersion),
			VK_VERSION_MINOR(deviceProperties[GPUIndex].apiVersion),
			VK_VERSION_PATCH(deviceProperties[GPUIndex].apiVersion));

		GPUIndex++;
	}
	printf("----------------------\n");
	GPUIndex = 0;
	u16 pickedGPU = 0;
	bool32 vulkan_1_1 = false;
	bool32 vulkan_1_0 = false;


	for (const VkPhysicalDeviceProperties& props : deviceProperties)
	{
		VulkanQueueFamilyIndices queueFamilyIndices = vulkan_getQueueFamilyIndices(physicalDevices[GPUIndex]);

		VkBool32 surfaceSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[GPUIndex], queueFamilyIndices.graphics, surface, &surfaceSupport);

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			props.apiVersion >= VK_API_VERSION_1_1 &&
			surfaceSupport &&
			supportedFeatures[GPUIndex].samplerAnisotropy)
		{
			pickedGPU = GPUIndex;
			vulkan_1_1 = true;
			break;
		}
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
			props.apiVersion >= VK_API_VERSION_1_1 &&
			surfaceSupport &&
			supportedFeatures[GPUIndex].samplerAnisotropy)
		{
			pickedGPU = GPUIndex;
			vulkan_1_1 = true;
		}

		GPUIndex++;
	}

	if (!vulkan_1_1)
	{
		GPUIndex = 0;
		for (const VkPhysicalDeviceProperties& props : deviceProperties)
		{
			VulkanQueueFamilyIndices queueFamilyIndices = vulkan_getQueueFamilyIndices(physicalDevices[GPUIndex]);

			VkBool32 surfaceSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[GPUIndex], queueFamilyIndices.graphics, surface, &surfaceSupport);
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
				props.apiVersion >= VK_API_VERSION_1_0 &&
				surfaceSupport &&
				supportedFeatures[GPUIndex].samplerAnisotropy)
			{
				pickedGPU = GPUIndex;
				vulkan_1_0 = true;
				break;
			}
			else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
				props.apiVersion >= VK_API_VERSION_1_0 &&
				surfaceSupport &&
				supportedFeatures[GPUIndex].samplerAnisotropy)
			{
				pickedGPU = GPUIndex;
				vulkan_1_0 = true;
			}

			GPUIndex++;
		}
	}

	assert(vulkan_1_1 || vulkan_1_0);

	printf("Using device: %s with API version: %u.%u.%u\n",
		deviceProperties[pickedGPU].deviceName,
		VK_VERSION_MAJOR(deviceProperties[pickedGPU].apiVersion),
		VK_VERSION_MINOR(deviceProperties[pickedGPU].apiVersion),
		VK_VERSION_PATCH(deviceProperties[pickedGPU].apiVersion));
	printf("----------------------\n");

	VkPhysicalDevice pickedPhysicalDevice = physicalDevices[pickedGPU];

	return pickedPhysicalDevice;
}

u32 vulkan_findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	for (u32 i = 0; i < memoryProperties.memoryTypeCount; i++)
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	assert(!"Error");
	return ~0u;
}

VulkanPhysicalDeviceDescription vulkan_getPhysicalDeviceDescription(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VulkanPhysicalDeviceDescription description = {};
	vkGetPhysicalDeviceProperties(physicalDevice, &description.properties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &description.features);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &description.memoryProperties);

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	description.queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, description.queueFamilyProperties.data());

	u32 extensionCount = 0;
	VKCHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr));
	assert(extensionCount > 0);
	vector<VkExtensionProperties> extensionProperties(extensionCount);
	VKCHECK(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensionProperties.data()));
	description.extensions.resize(extensionCount);
	u32 i = 0;
	for (const VkExtensionProperties& extension : extensionProperties)
	{
		description.extensions[i] = extension.extensionName;
		i++;
	}

	description.deviceQueueConfiguration = vulkan_setupQueueCreation(description.queueFamilyProperties, description.queueFamilyIndices, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

	return description;
}

static bool32 vulkan_extensionSupported(const char* extension, const vector<string>& extensions)
{
	for (const auto& ext : extensions)
		if (strcmp(ext.c_str(), extension) == 0)
			return true;

	return false;
}

VkDevice vulkan_createDevice(VkPhysicalDevice physicalDevice, const VulkanPhysicalDeviceDescription& physicalDeviceDescription)
{
	vector<const char*> deviceExtensions;
	deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef _DEBUG
	if (vulkan_extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, physicalDeviceDescription.extensions))
	{
		deviceExtensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}
#endif

	vector<const char*> layers =
	{
#ifdef _DEBUG
		VK_KHR_VALIDATION_LAYER_NAME
#endif
	};

	VkDeviceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.queueCreateInfoCount = (u32)physicalDeviceDescription.deviceQueueConfiguration.size();
	createInfo.pQueueCreateInfos = physicalDeviceDescription.deviceQueueConfiguration.data();
	createInfo.enabledLayerCount = (u32)layers.size();
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.enabledExtensionCount = (u32)deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pEnabledFeatures = &physicalDeviceDescription.features;

	VkDevice device = nullptr;
	VKCHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device));

	return device;
}

VkSurfaceFormatKHR vulkan_pickSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	u32 formatCount = 0;
	VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
	assert(formatCount > 0);

	vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	VKCHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()));

	VkFormat colorFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	VkSurfaceFormatKHR pickedSurfaceFormat = {};
	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		pickedSurfaceFormat.format = VK_FORMAT_UNDEFINED;
		pickedSurfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		const VkFormat desiredFormat = VK_FORMAT_B8G8R8A8_UNORM;
		bool32 found_B8G8R8A8_UNORM = false;
		for (VkSurfaceFormatKHR& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == desiredFormat)
			{
				pickedSurfaceFormat.format = surfaceFormat.format;
				pickedSurfaceFormat.colorSpace = surfaceFormat.colorSpace;
				found_B8G8R8A8_UNORM = true;
				break;
			}
		}
		if (!found_B8G8R8A8_UNORM)
		{
			pickedSurfaceFormat.format = surfaceFormats[0].format;
			pickedSurfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
		}
	}

	return pickedSurfaceFormat;
}

VulkanSwapchain vulkan_createSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, u32 width, u32 height, VulkanSwapchain* oldSwapchain = nullptr)
{
	VulkanSwapchain swapchain;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VKCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

	u32 presentModeCount;
	VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
	assert(presentModeCount > 0);

	vector<VkPresentModeKHR> presentModes(presentModeCount);
	VKCHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));

	swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (u32 i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchain.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchain.presentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchain.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	u32 desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount)
	{
		desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// Non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfaceCapabilities.currentTransform;
	}

	// Find a supported composite alpha format (not all devices support alpha opaque)
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Simply select the first composite alpha format available
	vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags =
	{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (VkCompositeAlphaFlagBitsKHR& compositeAlphaFlag : compositeAlphaFlags)
	{
		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
		{
			compositeAlpha = compositeAlphaFlag;
			break;
		}
	}

	swapchain.surfaceFormat = vulkan_pickSurfaceFormat(physicalDevice, surface);

	swapchain.extent.width = width;
	swapchain.extent.height = height;

	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = desiredNumberOfSwapchainImages;
	createInfo.imageFormat = swapchain.surfaceFormat.format;
	createInfo.imageColorSpace = swapchain.surfaceFormat.colorSpace;
	createInfo.imageExtent.width = swapchain.extent.width;
	createInfo.imageExtent.height = swapchain.extent.height;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	createInfo.compositeAlpha = compositeAlpha;
	createInfo.presentMode = swapchain.presentMode;
	createInfo.clipped = true;
	createInfo.oldSwapchain = oldSwapchain ? oldSwapchain->handle : nullptr;

	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	{
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VKCHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain.handle));

	if (oldSwapchain && oldSwapchain->handle)
	{
		for (VkImageView imageView : oldSwapchain->imageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(device, oldSwapchain->handle, nullptr);
	}

	u32 imageCount;
	VKCHECK(vkGetSwapchainImagesKHR(device, swapchain.handle, &imageCount, nullptr));
	assert(imageCount > 0);
	swapchain.images.resize(imageCount);
	swapchain.imageViews.resize(imageCount);
	VKCHECK(vkGetSwapchainImagesKHR(device, swapchain.handle, &imageCount, swapchain.images.data()));

	for (u32 i = 0; i < imageCount; i++)
	{
		VkImageViewCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.image = swapchain.images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchain.surfaceFormat.format;
		createInfo.components =
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VKCHECK(vkCreateImageView(device, &createInfo, nullptr, &swapchain.imageViews[i]));
	}

	return swapchain;
}

VkCommandPool vulkan_createCommandPool(VkDevice device, u32 queueIndex)
{
	VkCommandPoolCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.queueFamilyIndex = queueIndex;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCommandPool commandPool = nullptr;
	VKCHECK(vkCreateCommandPool(device, &createInfo, nullptr, &commandPool));

	return commandPool;
}

VkSampleCountFlagBits vulkan_pickSampleCount(const VkPhysicalDeviceProperties& physicalDeviceProperties)
{
	VkSampleCountFlags counts = min(physicalDeviceProperties.limits.framebufferColorSampleCounts, physicalDeviceProperties.limits.framebufferDepthSampleCounts);
#if 0
	{
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	}
#endif

	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}

VkImage vulkan_createImage(VkDevice device, VkExtent3D extent, u32 mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
{
	VkImageCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = format;
	createInfo.extent.width = extent.width;
	createInfo.extent.height = extent.height;
	createInfo.extent.depth = extent.depth;
	createInfo.mipLevels = mipLevels;
	createInfo.arrayLayers = 1;
	createInfo.samples = samples;
	createInfo.tiling = tiling;
	createInfo.usage = usage;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image = nullptr;
	VKCHECK(vkCreateImage(device, &createInfo, nullptr, &image));

	return image;
}

VkDeviceMemory vulkan_allocateMemoryForImage(VkDevice device, VkImage image, VkMemoryPropertyFlags memoryPropertyFlags, VkPhysicalDeviceMemoryProperties memoryProperties)
{
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = vulkan_findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags, memoryProperties);

	VkDeviceMemory memory = nullptr;
	VKCHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &memory));

	VKCHECK(vkBindImageMemory(device, image, memory, 0));

	return memory;
}


VkImageView vulkan_createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, u32 mipLevels)
{
	VkImageViewCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = mipLevels;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = nullptr;
	VKCHECK(vkCreateImageView(device, &createInfo, nullptr, &imageView));

	return imageView;
}

VkCommandBuffer vulkan_beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = nullptr;
	VKCHECK(vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VKCHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	return commandBuffer;
}

void vulkan_endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
	VKCHECK(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VKCHECK(vkQueueSubmit(queue, 1, &submitInfo, nullptr));
	VKCHECK(vkQueueWaitIdle(queue));

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

static inline bool32 vulkan_hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void vulkan_transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, u32 mipLevels)
{
	VkCommandBuffer	transferCommandBuffer = vulkan_beginSingleTimeCommands(device, commandPool);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = 0; // TODO: CHANGE
	barrier.dstAccessMask = 0; // TODO: CHANGE
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (vulkan_hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else
	{
		assert(!"Unsupported transition");
	}

	vkCmdPipelineBarrier(transferCommandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	vulkan_endSingleTimeCommands(device, commandPool, transferCommandBuffer, queue);
}

VulkanMSAA vulkan_createMultisamplingBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkPhysicalDeviceProperties& physicalDeviceProperties, const VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties, VkFormat swapchainImageFormat, const VkExtent2D& swapchainExtent, u32 mipLevels)
{
	VulkanMSAA msaa;
	msaa.samples = vulkan_pickSampleCount(physicalDeviceProperties);
	msaa.image = vulkan_createImage(device, { swapchainExtent.width, swapchainExtent.height, 1}, mipLevels, msaa.samples, swapchainImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	msaa.memory = vulkan_allocateMemoryForImage(device, msaa.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDeviceMemoryProperties);
	msaa.view = vulkan_createImageView(device, msaa.image, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	vulkan_transitionImageLayout(device, commandPool, queue, msaa.image, swapchainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

	return msaa;
}

VkShaderModule vulkan_createShaderModule(VkDevice device, const char* path)
{
	FILE* file = fopen(path, "rb"); // read binary flag
	assert(file);

	fseek(file, 0, SEEK_END);
	long length = ftell(file); // length (bytes) of the shader bytecode
	assert(length >= 0);
	fseek(file, 0, SEEK_SET);

	string buffer;
	buffer.reserve(length);

	size_t rc = fread(&buffer[0], 1, length, file);
	assert(rc == size_t(length));
	fclose(file);

	VkShaderModuleCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.codeSize = length; // note: this needs to be a number of bytes!
	createInfo.pCode = reinterpret_cast<const u32*>(buffer.c_str());

	VkShaderModule shaderModule = nullptr;
	VKCHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	assert(shaderModule);
	assert(length % 4 == 0);

	return shaderModule;
}

VkDescriptorSetLayout vulkan_createDescriptorSetLayout(VkDevice device)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding;
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding;
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding layoutBindings[2] = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.bindingCount = ARRAYSIZE(layoutBindings);
	createInfo.pBindings = layoutBindings;

	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	VKCHECK(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout));

	return descriptorSetLayout;
}

VkPipelineLayout vulkan_createPipelineLayout(VkDevice device, VkDescriptorSetLayout* pDescriptorSetLayout)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = pDescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	VkPipelineLayout pipelineLayout = nullptr;
	VKCHECK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

	return pipelineLayout;
}

VkRenderPass vulkan_createRenderPass(VkDevice device, VkFormat swapchainFormat, VkFormat depthFormat, VkSampleCountFlagBits sampleCount)
{
	VkAttachmentDescription colorAttachment;
	colorAttachment.flags = 0;
	colorAttachment.format = swapchainFormat;
	colorAttachment.samples = sampleCount;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // multisampled images cannot be presented directly!! 

	VkAttachmentDescription depthAttachment;
	depthAttachment.flags = 0;
	depthAttachment.format = depthFormat;
	depthAttachment.samples = sampleCount;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve;
	colorAttachmentResolve.flags = 0;
	colorAttachmentResolve.format = swapchainFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference;
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference;
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveReference;
	colorAttachmentResolveReference.attachment = 2;
	colorAttachmentResolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pResolveAttachments = &colorAttachmentResolveReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	VkSubpassDependency subpassDependency;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;


	array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	VkRenderPassCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.attachmentCount = u32(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDescription;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &subpassDependency;

	VkRenderPass renderPass = nullptr;
	VKCHECK(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));

	return renderPass;
}

// Helpers
static inline VkVertexInputBindingDescription vulkan_getBindingDescription()
{
	VkVertexInputBindingDescription inputBindingDescription;
	inputBindingDescription.binding = 0;
	inputBindingDescription.stride = sizeof(Vertex);
	inputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return inputBindingDescription;
}

static inline array<VkVertexInputAttributeDescription, VERTEX_ATTRIBUTES> vulkan_getAttributeDescriptions()
{
	array<VkVertexInputAttributeDescription, VERTEX_ATTRIBUTES> attributeDescriptions;
	// First attribute: position
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);
	// Second attribute: color
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);
	// Third attribute: texture coordinates
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	return attributeDescriptions;
}

static inline VkPipelineShaderStageCreateInfo vulkan_createShaderPipelineStage(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStage, const char* entryPoint = "main")
{
	VkPipelineShaderStageCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = shaderStage;
	createInfo.module = shaderModule;
	createInfo.pName = entryPoint;
	createInfo.pSpecializationInfo = nullptr;

	return createInfo;
}

VkPipeline vulkan_createGraphicsPipeline(VkDevice device, VkShaderModule vertexShader, VkShaderModule fragmentShader, const VkExtent2D& swapchainExtent, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkSampleCountFlagBits sampleCount)
{
	VkPipelineShaderStageCreateInfo vertexShaderStage = vulkan_createShaderPipelineStage(vertexShader, VK_SHADER_STAGE_VERTEX_BIT);
	VkPipelineShaderStageCreateInfo fragmentShaderStage = vulkan_createShaderPipelineStage(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStage, fragmentShaderStage };

	VkVertexInputBindingDescription bindingDescription = vulkan_getBindingDescription();
	array<VkVertexInputAttributeDescription, VERTEX_ATTRIBUTES> attributeDescriptions = vulkan_getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputState;
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext = nullptr;
	vertexInputState.flags = 0;
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.pVertexBindingDescriptions = &bindingDescription;
	vertexInputState.vertexAttributeDescriptionCount = u32(attributeDescriptions.size());
	vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.pNext = nullptr;
	inputAssemblyState.flags = 0;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.primitiveRestartEnable = false;

	VkViewport viewport;
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.width = float(swapchainExtent.width);
	viewport.height = float(swapchainExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = swapchainExtent.width;
	scissor.extent.height = swapchainExtent.height;

	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationState;
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.pNext = nullptr;
	rasterizationState.flags = 0;
	// useful for shadow maps, require GPU feature
	rasterizationState.depthClampEnable = false;
	// pass geometry to the framebuffer
	rasterizationState.rasterizerDiscardEnable = false;
	// how fragments are generated for geometry (normal mode). The others require GPU feature
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;

	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = false;
	rasterizationState.depthBiasConstantFactor = 0.f;
	rasterizationState.depthBiasClamp = 0.f;
	rasterizationState.depthBiasSlopeFactor = 0.f;
	rasterizationState.lineWidth = 1.f;

	VkPipelineMultisampleStateCreateInfo multisampleState;
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pNext = nullptr;
	multisampleState.flags = 0;
	multisampleState.rasterizationSamples = sampleCount;
	multisampleState.sampleShadingEnable = true;
	multisampleState.minSampleShading = 1.f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = false;
	multisampleState.alphaToOneEnable = false;

	// Depth/stencil
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.pNext = nullptr;
	depthStencilState.flags = 0;
	depthStencilState.depthTestEnable = true;
	depthStencilState.depthWriteEnable = true;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = false;
	depthStencilState.stencilTestEnable = false;
	depthStencilState.front = {};
	depthStencilState.back = {};
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
	colorBlendAttachmentState.blendEnable = false;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState;
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pNext = nullptr;
	colorBlendState.flags = 0;
	colorBlendState.logicOpEnable = false;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;
	colorBlendState.blendConstants[0] = 0.f;
	colorBlendState.blendConstants[1] = 0.f;
	colorBlendState.blendConstants[2] = 0.f;
	colorBlendState.blendConstants[3] = 0.f;

	/*
	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	*/

	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = nullptr;
	dynamicState.flags = 0;
	dynamicState.dynamicStateCount = 0;
	dynamicState.pDynamicStates = nullptr;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext = nullptr;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	graphicsPipelineCreateInfo.pTessellationState = nullptr;
	graphicsPipelineCreateInfo.pViewportState = &viewportState;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilState;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = nullptr;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	VkPipeline graphicsPipeline = nullptr;
	VKCHECK(vkCreateGraphicsPipelines(device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline));

	return graphicsPipeline;
}

VulkanDepthStencil vulkan_createDepthStencil(VkDevice device, VkPhysicalDevice physicalDevice, VkExtent2D const& extent, VkPhysicalDeviceMemoryProperties const& memoryProperties)
{
	VulkanDepthStencil depthStencil;
	VkFormat depthFormat = VK_FORMAT_UNDEFINED;
	array<VkFormat, 5> depthFormats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (VkFormat format : depthFormats)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = format;
		}
	}
	assert(depthFormat != VK_FORMAT_UNDEFINED);
	depthStencil.depthFormat = depthFormat;

	{
		VkImageCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = depthFormat;
		createInfo.extent.width = extent.width;
		createInfo.extent.height = extent.height;
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.samples = VK_SAMPLE_COUNT_4_BIT;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VKCHECK(vkCreateImage(device, &createInfo, nullptr, &depthStencil.image));
	}

	{
		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, depthStencil.image, &memoryRequirements);

		VkMemoryAllocateInfo allocateInfo;
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = vulkan_findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryProperties);

		VKCHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &depthStencil.memory));
		VKCHECK(vkBindImageMemory(device, depthStencil.image, depthStencil.memory, 0));
	}

	{
		VkImageViewCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.image = depthStencil.image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = depthStencil.depthFormat;
		createInfo.components = {};
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (depthStencil.depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
		{
			createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		VKCHECK(vkCreateImageView(device, &createInfo, nullptr, &depthStencil.imageView));
	}

	return depthStencil;
}

vector<VkFramebuffer> vulkan_createFramebuffers(VkDevice device, const vector<VkImageView>& imageViews, const VkExtent2D& swapchainExtent, VkRenderPass renderPass, VkImageView depthImageView, VkImageView colorImageView)
{
	array<VkImageView, 3> attachments = { colorImageView, depthImageView, VkImageView() };

	VkFramebufferCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = u32(attachments.size());
	createInfo.width = swapchainExtent.width;
	createInfo.height = swapchainExtent.height;
	createInfo.layers = 1;

	u32 imageViewCount = u32(imageViews.size());
	vector<VkFramebuffer> framebuffers(imageViewCount);
	for (u32 i = 0; i < imageViewCount; i++)
	{
		attachments[2] = imageViews[i];
		createInfo.pAttachments = attachments.data();
		VKCHECK(vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffers[i]));
	}

	return framebuffers;
}

vector<VkCommandBuffer> vulkan_createCommandBuffers(VkDevice device, VkCommandPool commandPool, u32 swapchainImageCount)
{
	VkCommandBufferAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = swapchainImageCount;

	vector<VkCommandBuffer> commandBuffers(swapchainImageCount);
	VKCHECK(vkAllocateCommandBuffers(device, &allocateInfo, commandBuffers.data()));

	return commandBuffers;
}

static inline void vulkan_copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage dstImage, VkBuffer srcBuffer, VkExtent2D imageExtent)
{
	VkCommandBuffer transferCommandBuffer = vulkan_beginSingleTimeCommands(device, commandPool);

	VkBufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent.width = imageExtent.width;
	region.imageExtent.height = imageExtent.height;
	region.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	vulkan_endSingleTimeCommands(device, commandPool, transferCommandBuffer, queue);
}

VkSampler vulkan_createTextureSampler(VkDevice device, u32 mipLevels)
{
	VkSamplerCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.mipLodBias = 0.0f;
	createInfo.anisotropyEnable = true;
	createInfo.maxAnisotropy = 16;
	createInfo.compareEnable = false;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.minLod = 0.0f;
	createInfo.maxLod = float(mipLevels);
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = false;

	VkSampler sampler = nullptr;
	VKCHECK(vkCreateSampler(device, &createInfo, nullptr, &sampler));

	return sampler;
}

void vulkan_generateMipmaps(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat imageFormat, int textureWidth, int textureHeight, u32 mipLevels)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
	assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);

	VkCommandBuffer commandBuffer = vulkan_beginSingleTimeCommands(device, commandPool);

	int mipWidth = textureWidth;
	int mipHeight = textureHeight;

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcQueueFamilyIndex = 0;
	imageMemoryBarrier.dstQueueFamilyIndex = 0;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	for (u32 i = 0; i < mipLevels - 1; i++)
	{
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = i;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		VkImageBlit imageBlit;
		imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.srcSubresource.mipLevel = i;
		imageBlit.srcSubresource.baseArrayLayer = 0;
		imageBlit.srcSubresource.layerCount = 1;
		imageBlit.srcOffsets[0] = { 0, 0, 0 };
		imageBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlit.dstSubresource.mipLevel = i + 1;
		imageBlit.dstSubresource.baseArrayLayer = 0;
		imageBlit.dstSubresource.layerCount = 1;
		imageBlit.dstOffsets[0] = { 0, 0, 0 };
		imageBlit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	vulkan_endSingleTimeCommands(device, commandPool, commandBuffer, queue);
}

static inline VkBuffer vulkan_createBuffer(VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, const VulkanQueueInfo& queueInfo)
{
	VkBufferCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.size = bufferSize;
	createInfo.usage = usage;
	createInfo.sharingMode = queueInfo.sharingMode;
	createInfo.queueFamilyIndexCount = queueInfo.queueFamilyIndexCount;
	createInfo.pQueueFamilyIndices = queueInfo.pQueueFamilyIndices;

	VkBuffer buffer = nullptr;
	VKCHECK(vkCreateBuffer(device, &createInfo, nullptr, &buffer));

	return buffer;
}

static inline VkDeviceMemory vulkan_allocateMemoryForBuffer(VkDevice device, VkBuffer buffer, const VkPhysicalDeviceMemoryProperties& memoryProperties, VkMemoryPropertyFlags memoryFlags)
{
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = vulkan_findMemoryType(memoryRequirements.memoryTypeBits, memoryFlags, memoryProperties);

	VkDeviceMemory bufferMemory = nullptr;
	VKCHECK(vkAllocateMemory(device, &allocateInfo, nullptr, &bufferMemory));

	VKCHECK(vkBindBufferMemory(device, buffer, bufferMemory, 0));

	return bufferMemory;
}

void vulkan_copyDataToMemory(VkDevice device, VkDeviceMemory memory, u64 dataSize, const void* dataToCopy)
{
	void* data;
	VKCHECK(vkMapMemory(device, memory, 0, dataSize, 0, &data));
	memcpy(data, dataToCopy, dataSize);
	vkUnmapMemory(device, memory);
}

void vulkan_copyBuffer(VkDevice device, VkCommandPool transferCommandPool, VkQueue transferQueue,
	VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer transferCommandBuffer = vulkan_beginSingleTimeCommands(device, transferCommandPool);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vulkan_endSingleTimeCommands(device, transferCommandPool, transferCommandBuffer, transferQueue);
}

VulkanBuffer vulkan_createStagingBuffer(VkDevice device, const void* bufferData, u64 bufferSize, const VulkanQueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	VulkanBuffer stagingBuffer;

	stagingBuffer.handle = vulkan_createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, queueInfo);
	stagingBuffer.memory = vulkan_allocateMemoryForBuffer(device, stagingBuffer.handle, memoryProperties, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	vulkan_copyDataToMemory(device, stagingBuffer.memory, bufferSize, bufferData);

	return stagingBuffer;
}

VulkanBuffer vulkan_createLocalDeviceBuffer(VkDevice device, u64 bufferSize, VkBufferUsageFlags bufferUsage, const VulkanQueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	VulkanBuffer deviceBuffer = {};

	deviceBuffer.handle = vulkan_createBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage, queueInfo);
	deviceBuffer.memory = vulkan_allocateMemoryForBuffer(device, deviceBuffer.handle, memoryProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	return deviceBuffer;
}

VulkanBuffer vulkan_bufferDataIntoLocalDevice(VkDevice device, const void* bufferData, u64 bufferSize, VkBufferUsageFlags bufferUsage, VkCommandPool transferCommandPool, VkQueue transferQueue, const VulkanQueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	VulkanBuffer stagingBuffer = vulkan_createStagingBuffer(device, bufferData, bufferSize, queueInfo, memoryProperties);
	VulkanBuffer deviceBuffer = vulkan_createLocalDeviceBuffer(device, bufferSize, bufferUsage, queueInfo, memoryProperties);

	// Copy data to the buffer
	// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is slower than using vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges
	// vkFlushMappedMemoryRanges must be called after writing to the mapped memory
	// vkInvalidateMappedMemoryRanges must be called before reading from the mapped memory
	// but actually we're copying to the local device, so it should be fast

	vulkan_copyDataToMemory(device, stagingBuffer.memory, bufferSize, bufferData);

	vulkan_copyBuffer(device, transferCommandPool, transferQueue, stagingBuffer.handle, deviceBuffer.handle, bufferSize);

	vkDestroyBuffer(device, stagingBuffer.handle, nullptr);
	vkFreeMemory(device, stagingBuffer.memory, nullptr);

	return deviceBuffer;
}

VulkanBufferList vulkan_createUniformBuffers(VkDevice device, VkDeviceSize swapchainImageCount, const VulkanQueueInfo& queueInfo, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	VulkanBufferList uniformBuffers;
	uniformBuffers.handle.resize(swapchainImageCount);
	uniformBuffers.memory.resize(swapchainImageCount);

	for (size_t i = 0; i < swapchainImageCount; i++)
	{
		uniformBuffers.handle[i] = vulkan_createBuffer(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, queueInfo);
		uniformBuffers.memory[i] = vulkan_allocateMemoryForBuffer(device, uniformBuffers.handle[i], memoryProperties, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	return uniformBuffers;
}

void vulkan_updateUniformBuffer(VkDevice device, const VkExtent2D& swapchainExtent, VkDeviceMemory uboMemory, float t)
{
	UniformBufferObject ubo;
	ubo.model = glm::mat4(1.0f);
	ubo.view = glm::mat4(1.0f);
	ubo.proj = glm::mat4(1.0f);
	
	ubo.model = glm::scale(ubo.model, glm::vec3(4.5f, 4.5f, 4.5f));
	ubo.model = glm::rotate(ubo.model, glm::radians(135.f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.model = glm::rotate(ubo.model, glm::radians(65.f), glm::vec3(1.0f, 0.0f, 0.0f));
	ubo.model = glm::rotate(ubo.model, glm::radians(45.f), glm::vec3(0.0f, 1.0f, 0.0f));
	//glm::mat4 rot = glm::rotate(glm::mat4(1.0f), t * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.model = rot * ubo.model;
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.f), (float)swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f);
	// Invert to prevent image to be rendered upside down
	ubo.proj[1][1] *= -1.0f;

	vulkan_copyDataToMemory(device, uboMemory, sizeof(ubo), &ubo);
}

VulkanTexture vulkan_loadTexture(const char* texturePath, VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue, const VkPhysicalDeviceMemoryProperties& memoryProperties, VkSampleCountFlagBits samples, VkFormat textureFormat = VK_FORMAT_R8G8B8A8_UNORM, VkImageTiling tilingMode = VK_IMAGE_TILING_OPTIMAL, VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, const VulkanQueueInfo& queueInfo = { nullptr, 0, VK_SHARING_MODE_EXCLUSIVE })
{
	int textureWidth;
	int textureHeight;
	int textureChannels;

	stbi_uc* texturePixels = stbi_load(texturePath, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
	assert(texturePixels);
	VkDeviceSize textureSize = (u64)textureWidth * (u64)textureHeight * 4;
	VkExtent2D textureExtent = { (u32)textureWidth, (u32)textureHeight};

	VulkanBuffer stagingBuffer = vulkan_createStagingBuffer(device, texturePixels, textureSize, queueInfo, memoryProperties);
	stbi_image_free(texturePixels);

	VulkanTexture texture;
	texture.mipLevels = u32(floor(log2(max(textureWidth, textureHeight)))) + 1;
	texture.handle = vulkan_createImage(device, { textureExtent.width, textureExtent.height, 1 }, texture.mipLevels, samples, textureFormat, tilingMode, imageUsage);
	texture.memory = vulkan_allocateMemoryForImage(device, texture.handle, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryProperties);

	vulkan_transitionImageLayout(device, commandPool, queue, texture.handle, textureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);
	vulkan_copyBufferToImage(device, commandPool, queue, texture.handle, stagingBuffer.handle, textureExtent);
	vulkan_generateMipmaps(physicalDevice, device, commandPool, queue, texture.handle, textureFormat, (i32)textureExtent.width, (i32)textureExtent.height, texture.mipLevels);

	vkDestroyBuffer(device, stagingBuffer.handle, nullptr);
	vkFreeMemory(device, stagingBuffer.memory, nullptr);

	texture.view = vulkan_createImageView(device, texture.handle, textureFormat, VK_IMAGE_ASPECT_COLOR_BIT, texture.mipLevels);
	texture.sampler = vulkan_createTextureSampler(device, texture.mipLevels);

	return texture;
}

VkDescriptorPool vulkan_createDescriptorPool(VkDevice device, u32 swapchainImageCount)
{
	VkDescriptorPoolSize descriptorPoolSizes[2];
	descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSizes[0].descriptorCount = swapchainImageCount;
	descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSizes[1].descriptorCount = swapchainImageCount;

	VkDescriptorPoolCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	createInfo.maxSets = swapchainImageCount;
	createInfo.poolSizeCount = ARRAYSIZE(descriptorPoolSizes);
	createInfo.pPoolSizes = descriptorPoolSizes;

	VkDescriptorPool descriptorPool = nullptr;
	VKCHECK(vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool));

	return descriptorPool;
}

vector<VkDescriptorSet> vulkan_createDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, u32 swapchainImageCount, VkDescriptorSetLayout descriptorSetLayout, const
	VulkanBufferList& uniformBuffers, VkImageView imageView, VkSampler sampler)
{
	vector<VkDescriptorSetLayout> descriptorSetLayouts(swapchainImageCount, descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = swapchainImageCount;
	allocateInfo.pSetLayouts = descriptorSetLayouts.data();

	vector<VkDescriptorSet> descriptorSets(swapchainImageCount);

	VKCHECK(vkAllocateDescriptorSets(device, &allocateInfo, descriptorSets.data()));

	VkDescriptorImageInfo imageInfo;
	imageInfo.sampler = sampler;
	imageInfo.imageView = imageView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	for (u32 i = 0; i < swapchainImageCount; i++)
	{
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffers.handle[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet descriptorWrites[2];
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].pNext = nullptr;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pTexelBufferView = nullptr;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].pNext = nullptr;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device, ARRAYSIZE(descriptorWrites), descriptorWrites, 0, nullptr);
	}

	return descriptorSets;
}

void vulkan_buildCommandBuffers(VkRenderPass renderPass, const VkExtent2D& swapchainExtent, const vector<VkCommandBuffer>& commandBuffers, const vector<VkFramebuffer>& framebuffers, VkBuffer& vertexBuffer, VkBuffer& indexBuffer, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, const vector<Vertex>& vertices, const vector<u32>& indices, const vector<VkDescriptorSet>& descriptorSets)
{
	VkCommandBufferBeginInfo beginInfo;
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.f, 0.f, 0.f, 1.f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo;
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent.width = swapchainExtent.width;
	renderPassInfo.renderArea.extent.height = swapchainExtent.height;
	renderPassInfo.clearValueCount = u32(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VKCHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

		renderPassInfo.framebuffer = framebuffers[i];

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);
#define INDEXSIZE 32
#if INDEXSIZE == 32
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
#elif INDEXSIZE == 16
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
#endif
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		vkCmdDrawIndexed(commandBuffers[i], (u32)indices.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		VKCHECK(vkEndCommandBuffer(commandBuffers[i]));
	}
}

inline VulkanFrameSynchronization vulkan_createSynchronizationResources(VkDevice device, const u32 maxFramesInFlight)
{
	VulkanFrameSynchronization fss;

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

inline void vulkan_updateCurrentFrame(VulkanFrameSynchronization& frameSync)
{
	frameSync.currentFrame = (frameSync.currentFrame + 1) % frameSync.maxFramesInFlight;
}

void vulkan_submitQueue(VkDevice device, VkQueue graphicsQueue, VkCommandBuffer drawCommandBuffer, VkFence* pInflightFence, VkSemaphore* pImageAcquireSemaphore, VkSemaphore* pImageReleaseSemaphore)
{
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = pImageAcquireSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &drawCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = pImageReleaseSemaphore;

	VKCHECK(vkResetFences(device, 1, pInflightFence));
	VKCHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, *pInflightFence));
}

VkResult vulkan_present(VkDevice device, VkSwapchainKHR swapchain, u32 currentFrame, VkQueue graphicsQueue, u32* pImageIndex, VkSemaphore* pImageReleaseSemaphore)
{
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = pImageReleaseSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = pImageIndex;
	presentInfo.pResults = nullptr;

	return vkQueuePresentKHR(graphicsQueue, &presentInfo);
}

SwapchainStatus vulkan_updateSwapchain(VulkanSwapchain& swapchain, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VKCHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));
	u32 newWidth = surfaceCapabilities.currentExtent.width;
	u32 newHeight = surfaceCapabilities.currentExtent.height;

	if (newWidth == 0 || newHeight == 0)
	{
		return SwapchainStatus::NOT_READY;
	}
	if (swapchain.extent.width == newWidth && swapchain.extent.height == newHeight)
	{
		return SwapchainStatus::READY;
	}
	
	VKCHECK(vkDeviceWaitIdle(device));
	swapchain = vulkan_createSwapchain(physicalDevice, device, surface, newWidth, newHeight, &swapchain);

	return SwapchainStatus::RESIZED;
}

void vulkan_onWindowResize(VulkanApplication* vk)
{
	vkDestroyImage(vk->device, vk->msaa.image, nullptr);
	vkDestroyImageView(vk->device, vk->msaa.view, nullptr);
	vkFreeMemory(vk->device, vk->msaa.memory, nullptr);
	vk->msaa = vulkan_createMultisamplingBuffer(vk->device, vk->graphicsCommandPool, vk->graphicsQueue, vk->deviceDescription.properties, vk->deviceDescription.memoryProperties, vk->swapchain.surfaceFormat.format, vk->swapchain.extent, 1);

	vkDestroyImageView(vk->device, vk->depthStencil.imageView, nullptr);
	vkDestroyImage(vk->device, vk->depthStencil.image, nullptr);
	vkFreeMemory(vk->device, vk->depthStencil.memory, nullptr);
	vk->depthStencil = vulkan_createDepthStencil(vk->device, vk->physicalDevice, vk->swapchain.extent, vk->deviceDescription.memoryProperties);
	
	vkDestroyRenderPass(vk->device, vk->renderPass, nullptr);
	vk->renderPass = vulkan_createRenderPass(vk->device, vk->swapchain.surfaceFormat.format, vk->depthStencil.depthFormat, vk->msaa.samples);
	for (VkFramebuffer framebuffer : vk->framebuffers)
	{
		vkDestroyFramebuffer(vk->device, framebuffer, nullptr);
	}
	vk->framebuffers = vulkan_createFramebuffers(vk->device, vk->swapchain.imageViews, vk->swapchain.extent, vk->renderPass, vk->depthStencil.imageView, vk->msaa.view);
	
	vkFreeCommandBuffers(vk->device, vk->graphicsCommandPool, u32(vk->drawCommandBuffers.size()), vk->drawCommandBuffers.data());
	vk->drawCommandBuffers = vulkan_createCommandBuffers(vk->device, vk->graphicsCommandPool, u32(vk->swapchain.images.size()));
	vkDestroyPipelineLayout(vk->device, vk->graphicsPipelineLayout, nullptr);
	vk->graphicsPipelineLayout = vulkan_createPipelineLayout(vk->device, &vk->descriptorSetLayout);
	vkDestroyPipeline(vk->device, vk->graphicsPipeline, nullptr);
	vk->graphicsPipeline = vulkan_createGraphicsPipeline(vk->device, vk->VS, vk->FS, vk->swapchain.extent, vk->graphicsPipelineLayout, vk->renderPass, vk->msaa.samples);
	for (VkBuffer uniformBuffer: vk->uniformBuffers.handle)
	{
		vkDestroyBuffer(vk->device, uniformBuffer, nullptr);
	}
	for (VkDeviceMemory ubMemory: vk->uniformBuffers.memory)
	{
		vkFreeMemory(vk->device, ubMemory, nullptr);
	}
	vk->uniformBuffers = vulkan_createUniformBuffers(vk->device, vk->swapchain.images.size(), { nullptr, 0, VK_SHARING_MODE_EXCLUSIVE }, vk->deviceDescription.memoryProperties);
	vkFreeDescriptorSets(vk->device, vk->descriptorPool, u32(vk->descriptorSets.size()), vk->descriptorSets.data());
	vk->descriptorSets = vulkan_createDescriptorSets(vk->device, vk->descriptorPool, u32(vk->swapchain.images.size()), vk->descriptorSetLayout, vk->uniformBuffers, vk->texture.view, vk->texture.sampler);
	vulkan_buildCommandBuffers(vk->renderPass, vk->swapchain.extent, vk->drawCommandBuffers, vk->framebuffers, vk->vertexBuffer.handle, vk->indexBuffer.handle, vk->graphicsPipeline, vk->graphicsPipelineLayout, vk->mesh.vertices, vk->mesh.indices, vk->descriptorSets);
}

void destroyVulkanApplication(VulkanApplication& vk)
{
	VKCHECK(vkDeviceWaitIdle(vk.device));

	vkDestroyImage(vk.device, vk.msaa.image, nullptr);
	vkDestroyImageView(vk.device, vk.msaa.view, nullptr);
	vkFreeMemory(vk.device, vk.msaa.memory, nullptr);

	vkDestroyImageView(vk.device, vk.depthStencil.imageView, nullptr);
	vkDestroyImage(vk.device, vk.depthStencil.image, nullptr);
	vkFreeMemory(vk.device, vk.depthStencil.memory, nullptr);

	vkDestroyRenderPass(vk.device, vk.renderPass, nullptr);
	for (VkFramebuffer framebuffer : vk.framebuffers)
	{
		vkDestroyFramebuffer(vk.device, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(vk.device, vk.graphicsCommandPool, u32(vk.drawCommandBuffers.size()), vk.drawCommandBuffers.data());
	vkDestroyPipelineLayout(vk.device, vk.graphicsPipelineLayout, nullptr);
	vkDestroyPipeline(vk.device, vk.graphicsPipeline, nullptr);
	vkFreeDescriptorSets(vk.device, vk.descriptorPool, u32(vk.descriptorSets.size()), vk.descriptorSets.data());
	vkDestroyDescriptorPool(vk.device, vk.descriptorPool, nullptr);
	vkDestroyCommandPool(vk.device, vk.graphicsCommandPool, nullptr);

	for (VkSemaphore semaphore : vk.frameSync.imageAcquireSemaphores)
	{
		vkDestroySemaphore(vk.device, semaphore, nullptr);
	}
	for (VkSemaphore semaphore : vk.frameSync.imageReleaseSemaphores)
	{
		vkDestroySemaphore(vk.device, semaphore, nullptr);
	}
	for (VkFence fence: vk.frameSync.inflightFences)
	{
		vkDestroyFence(vk.device, fence, nullptr);
	}
	for (VkFence fence : vk.waitFences)
	{
		vkDestroyFence(vk.device, fence, nullptr);
	}
	
	vkDestroyShaderModule(vk.device, vk.VS, nullptr);
	vkDestroyShaderModule(vk.device, vk.FS, nullptr);

	vkDestroyImage(vk.device, vk.texture.handle, nullptr);
	vkDestroyImageView(vk.device, vk.texture.view, nullptr);
	vkDestroySampler(vk.device, vk.texture.sampler, nullptr);
	vkFreeMemory(vk.device, vk.texture.memory, nullptr);

	vkDestroyBuffer(vk.device, vk.vertexBuffer.handle, nullptr);
	vkFreeMemory(vk.device, vk.vertexBuffer.memory, nullptr);
	vkDestroyBuffer(vk.device, vk.indexBuffer.handle, nullptr);
	vkFreeMemory(vk.device, vk.indexBuffer.memory, nullptr);

	for (VkDeviceMemory memory : vk.uniformBuffers.memory)
	{
		vkFreeMemory(vk.device, memory, nullptr);
	}
	for (VkBuffer buffer : vk.uniformBuffers.handle)
	{
		vkDestroyBuffer(vk.device, buffer, nullptr);
	}

	vkDestroyDescriptorSetLayout(vk.device, vk.descriptorSetLayout, nullptr);

	for (VkImageView swapchainImageView: vk.swapchain.imageViews)
	{
		vkDestroyImageView(vk.device, swapchainImageView, nullptr);
	}

	vkDestroySwapchainKHR(vk.device, vk.swapchain.handle, nullptr);
	vkDestroySurfaceKHR(vk.instance, vk.surface, nullptr);
	vkDestroyDevice(vk.device, nullptr);
	vkDestroyDebugReportCallbackEXT(vk.instance, vk.debugCallback, nullptr);
	vkDestroyInstance(vk.instance, nullptr);
}