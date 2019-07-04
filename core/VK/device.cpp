#include "../common.h"
#include "vulkan.h"
#include "queue.h"

#include "device.h"

static inline const constexpr bool useSwapchain = true;

#ifdef _DEBUG
VkBool32 defaultDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, u64 object,
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

VkDebugReportCallbackEXT createDebugCallback(VkInstance instance, VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT callback)
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

VkInstance createInstance()
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
#ifdef VOLK
	volkLoadInstance(instance);
#endif

	return instance;
}

PhysicalDeviceDescription getPhysicalDeviceDescription(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	PhysicalDeviceDescription description = {};
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

	description.deviceQueueConfiguration = setupQueueCreation(description.queueFamilyProperties, description.queueFamilyIndices, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

	return description;
}

static bool extensionSupported(const char* extension, const vector<string>& extensions)
{
	for (const auto& ext : extensions)
		if (strcmp(ext.c_str(), extension) == 0)
			return true;

	return false;
}

static const char* physicalDeviceTypeString(VkPhysicalDeviceType type)
{
	switch (type)
	{
#define STR(r) case VK_PHYSICAL_DEVICE_TYPE_ ##r: return #r
		STR(OTHER);
		STR(INTEGRATED_GPU);
		STR(DISCRETE_GPU);
		STR(VIRTUAL_GPU);
#undef STR
	default: return "UNKNOWN_DEVICE";
	}
}

VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
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
		printf("\tType: %s\n", physicalDeviceTypeString(deviceProperties[GPUIndex].deviceType));
		printf("\tAPI: %u.%u.%u\n",
			VK_VERSION_MAJOR(deviceProperties[GPUIndex].apiVersion),
			VK_VERSION_MINOR(deviceProperties[GPUIndex].apiVersion),
			VK_VERSION_PATCH(deviceProperties[GPUIndex].apiVersion));
		
		GPUIndex++;
	}
	printf("----------------------\n");
	GPUIndex = 0;
	u16 pickedGPU = 0;
	bool vulkan_1_1 = false;
	bool vulkan_1_0 = false;

	
	for (const VkPhysicalDeviceProperties& props : deviceProperties)
	{
		QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices(physicalDevices[GPUIndex]);

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
			QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices(physicalDevices[GPUIndex]);

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

u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties, const VkPhysicalDeviceMemoryProperties& memoryProperties)
{
	for (u32 i = 0; i < memoryProperties.memoryTypeCount; i++)
		if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	assert(!"Error");
	return ~0u;
}

VkDevice createDevice(VkPhysicalDevice physicalDevice, const PhysicalDeviceDescription& physicalDeviceDescription)
{
	vector<const char*> deviceExtensions;
	if (useSwapchain)
	{
		deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

#ifdef _DEBUG
	if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, physicalDeviceDescription.extensions))
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