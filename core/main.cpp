enum class RED_RENDERER_GRAPHICS_API : unsigned int
{
	NONE = 0x0,
	OPENGL = (1 << 1),
	DIRECTX_11 = (1 << 2),
	DIRECTX_12 = (1 << 3),
	VULKAN = (1 << 4),
};

#define OPENGL 0
#define DIRECTX11 0
#define DIRECTX12 0
#define VULKAN 1

#if VULKAN

#include "common.h"
#include "glfw.h"
#include "glm.h"
#include "tol.h"
#include "VK/sync.h"
#include "VK/queue.h"
#include "VK/device.h"
#include "VK/surface.h"
#include "VK/shader.h"
#include "VK/pipeline.h"
#include "VK/buffer.h"
#include "VK/descriptor.h"
#include "VK/image.h"
#include "VK/texture.h"
#include "VK/depth.h"
#include "VK/antialiasing.h"
#include "VK/swapchain.h"

#define APPLICATION_NAME "Red Renderer - A Vulkan 1.1 Renderer"
#define WIDTH 1024
#define HEIGHT 576
bool framebufferResized = false;
bool dedicatedTransferQueue = false;

const string rootDirectory = "../../";
const string shaderBytecodePath = "/shaders/bytecode/";
const string modelsDirectory = "core/models/";
const string texturesDirectory = "core/textures/";

const string vertexShaderBytecodeName = "triangle.vert.spv";
const string fragmentShaderBytecodeName = "triangle.frag.spv";

const string modelName = "chalet.obj";
const string textureName = "chalet.jpg";

const string vertexShaderFullPath = rootDirectory + shaderBytecodePath + vertexShaderBytecodeName;
const string fragmentShaderFullPath = rootDirectory + shaderBytecodePath + fragmentShaderBytecodeName;
const string modelFullPath = rootDirectory + modelsDirectory + modelName;
const string textureFullPath = rootDirectory + texturesDirectory + textureName;

int main(int argc, const char* argv[])
{
#ifdef GLFW
	assert(glfwInit());
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
#ifdef VOLK
	VKCHECK(volkInitialize());
#endif
	VkInstance instance = createInstance();
#ifdef _DEBUG
	VkDebugReportCallbackEXT debugCallback = createDebugCallback(instance,
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
		defaultDebugCallback);
#if _WIN64
	OutputDebugStringA("\n");
#endif
#endif
#ifdef GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, APPLICATION_NAME, nullptr, nullptr);
	glfwSetWindowUserPointer(window, &framebufferResized);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	// need GLFW_EXPOSE_NATIVE_WIN32 for glfwGetWin32Window()
	VkSurfaceKHR surface = createSurface(instance, GetModuleHandle(nullptr), glfwGetWin32Window(window));
#endif
	VkPhysicalDevice physicalDevice = pickPhysicalDevice(instance, surface);
	// WARNING: Possible queue family indices and queue configuration error
	PhysicalDeviceDescription physicalDeviceDescription = getPhysicalDeviceDescription(physicalDevice, surface);
	dedicatedTransferQueue = physicalDeviceDescription.queueFamilyIndices.graphics != physicalDeviceDescription.queueFamilyIndices.transfer;

	VkDevice device = createDevice(physicalDevice, physicalDeviceDescription);
	VkQueue graphicsQueue = nullptr;
	VkQueue transferQueue = nullptr;
	vkGetDeviceQueue(device, physicalDeviceDescription.queueFamilyIndices.graphics, 0, &graphicsQueue);
	if (dedicatedTransferQueue)
		vkGetDeviceQueue(device, physicalDeviceDescription.queueFamilyIndices.transfer, 0, &transferQueue);

	const u32 queueFamilyIndexArray[2] = { physicalDeviceDescription.queueFamilyIndices.graphics, physicalDeviceDescription.queueFamilyIndices.transfer };
	QueueInfo queueInfo = getQueueInfo(physicalDeviceDescription.queueFamilyIndices, queueFamilyIndexArray, ARRAYSIZE(queueFamilyIndexArray));

	SurfaceFeatures surfaceFeatures = getSurfaceFeatures(physicalDevice, surface);
	VkSurfaceFormatKHR swapchainFormatAndColor = pickSurfaceFormat(surfaceFeatures.formats);
	VkPresentModeKHR presentMode = pickPresentMode(surfaceFeatures.presentModes);
	VkExtent2D swapchainExtent = getSwapchainExtent(surfaceFeatures.capabilities, window);
	u32 swapchainMinImageCount = getSwapchainImageCount(surfaceFeatures.capabilities); // this function may be causing bugs
	VkSwapchainKHR swapchain = createSwapchain(physicalDevice, swapchainExtent, swapchainMinImageCount, swapchainFormatAndColor, presentMode, device, surface);

	vector<VkImage> swapchainImages = getSwapchainImages(device, swapchain);
	vector<VkImageView> imageViews = createImageViews(device, swapchainImages, swapchainFormatAndColor.format);
	
	VkCommandPool graphicsCommandPool = createCommandPool(device, physicalDeviceDescription.queueFamilyIndices.graphics);
	VkCommandPool transferCommandPool = createCommandPool(device, physicalDeviceDescription.queueFamilyIndices.transfer);

	MSAA msaa = createMultisamplingBuffer(device, graphicsCommandPool, graphicsQueue, physicalDeviceDescription.properties, physicalDeviceDescription.memoryProperties, swapchainFormatAndColor.format, swapchainExtent);

	VkShaderModule vertexShader = createShaderModule(device, vertexShaderFullPath.c_str());
	VkShaderModule fragmentShader = createShaderModule(device, fragmentShaderFullPath.c_str());
	VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout(device);
	VkPipelineLayout pipelineLayout = createPipelineLayout(device, &descriptorSetLayout);
	VkRenderPass renderPass = createRenderPass(device, swapchainFormatAndColor.format, findDepthFormat(physicalDevice), msaa.samples);

	
	VkPipeline graphicsPipeline = createGraphicsPipeline(device, vertexShader, fragmentShader, swapchainExtent, pipelineLayout, renderPass, msaa.samples);
	
	DepthBuffer depthBuffer = createDepthBuffer(physicalDevice, device, graphicsCommandPool, graphicsQueue, swapchainExtent, physicalDeviceDescription.memoryProperties, msaa.samples);
	vector<VkFramebuffer> framebuffers = createFramebuffers(device, imageViews, swapchainExtent, renderPass, depthBuffer.view, msaa.view);
	vector<VkCommandBuffer> graphicsCommandBuffers = createCommandBuffers(device, graphicsCommandPool, (u32)framebuffers.size());
	
	Texture texture = loadTexture(textureFullPath.c_str(), physicalDevice, device, graphicsCommandPool, graphicsQueue, physicalDeviceDescription.memoryProperties, VK_SAMPLE_COUNT_1_BIT);
	Mesh mesh = loadModel(modelFullPath.c_str());
	Buffer vertexBuffer = bufferDataIntoLocalDevice(device, mesh.vertices.data(), CONTAINER_BYTES(mesh.vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, transferCommandPool, transferQueue, queueInfo, physicalDeviceDescription.memoryProperties);
	Buffer indexBuffer = bufferDataIntoLocalDevice(device, mesh.indices.data(), CONTAINER_BYTES(mesh.indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, transferCommandPool, transferQueue, queueInfo, physicalDeviceDescription.memoryProperties);

	BufferList uniformBuffers = createUniformBuffers(device, swapchainImages.size(), queueInfo, physicalDeviceDescription.memoryProperties);
	VkDescriptorPool descriptorPool = createDescriptorPool(device, (u32)swapchainImages.size());
	vector<VkDescriptorSet> descriptorSets = createDescriptorSets(device, descriptorPool, u32(swapchainImages.size()), descriptorSetLayout, uniformBuffers, texture.view, texture.sampler);
	fillRenderPass(renderPass, swapchainExtent, graphicsCommandBuffers, framebuffers, vertexBuffer.handle, indexBuffer.handle, graphicsPipeline, pipelineLayout, mesh.vertices, mesh.indices, descriptorSets);

	FrameSyncStruct frameSync = createFrameSyncStruct(device, MAX_FRAMES_IN_FLIGHT);

#ifdef GLFW
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		u32 imageIndex = submitQueue(device, swapchain, swapchainExtent, frameSync.currentFrame, graphicsQueue, uniformBuffers, graphicsCommandBuffers.data(), frameSync.inflightFences.data(), &frameSync.imageAcquireSemaphores[frameSync.currentFrame], &frameSync.imageReleaseSemaphores[frameSync.currentFrame]);
		VkResult swapchainUpToDate = present(device, swapchain, frameSync.currentFrame, graphicsQueue, &imageIndex, &frameSync.imageReleaseSemaphores[frameSync.currentFrame]);
		if (swapchainUpToDate == VK_ERROR_OUT_OF_DATE_KHR || swapchainUpToDate == VK_SUBOPTIMAL_KHR || framebufferResized)
		{
			recreateSwapchain(swapchain, physicalDevice, device, surface, surfaceFeatures, window, queueInfo, framebuffers, descriptorPool, graphicsCommandBuffers,
				graphicsCommandPool, graphicsQueue, graphicsPipeline, pipelineLayout, renderPass, &descriptorSetLayout, uniformBuffers, physicalDeviceDescription.memoryProperties, physicalDeviceDescription.properties, swapchainImages, imageViews, vertexShader, fragmentShader, vertexBuffer.handle, indexBuffer.handle, mesh.vertices, mesh.indices, descriptorSets, texture.view, texture.sampler, depthBuffer, msaa);
			framebufferResized = false;
		}
		else
		{
			VKCHECK(swapchainUpToDate);
		}

		updateCurrentFrame(frameSync);
	}
#endif

	VKCHECK(vkDeviceWaitIdle(device));

	// Destroy Vulkan

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, frameSync.imageAcquireSemaphores[i], nullptr);
		vkDestroySemaphore(device, frameSync.imageReleaseSemaphores[i], nullptr);
		vkDestroyFence(device, frameSync.inflightFences[i], nullptr);
	}

	vkDestroyBuffer(device, vertexBuffer.handle, nullptr);
	vkFreeMemory(device, vertexBuffer.memory, nullptr);

	vkDestroyBuffer(device, indexBuffer.handle, nullptr);
	vkFreeMemory(device, indexBuffer.memory, nullptr);

	for (size_t i = 0; i < uniformBuffers.memory.size(); i++)
	{
		vkDestroyBuffer(device, uniformBuffers.handle[i], nullptr);
		vkFreeMemory(device, uniformBuffers.memory[i], nullptr);
	}

	vkDestroyImage(device, depthBuffer.image, nullptr);
	vkFreeMemory(device, depthBuffer.memory, nullptr);
	vkDestroyImageView(device, depthBuffer.view, nullptr);

	vkDestroyImage(device, msaa.image, nullptr);
	vkFreeMemory(device, msaa.memory, nullptr);
	vkDestroyImageView(device, msaa.view, nullptr);

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
	vkDestroyCommandPool(device, transferCommandPool, nullptr);

	for (const VkFramebuffer framebuffer: framebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	
	vkDestroyPipeline(device, graphicsPipeline, nullptr);

	vkDestroyImage(device, texture.handle, nullptr);
	vkFreeMemory(device, texture.memory, nullptr);
	vkDestroyImageView(device, texture.view, nullptr);
	vkDestroySampler(device, texture.sampler, nullptr);

	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyShaderModule(device, vertexShader, nullptr);
	vkDestroyShaderModule(device, fragmentShader, nullptr);
	

	for (const VkImageView imageView: imageViews)
		vkDestroyImageView(device, imageView, nullptr);

	vkDestroySwapchainKHR(device, swapchain, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	vkDestroyDevice(device, nullptr);
#ifdef _DEBUG
	vkDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
#endif
	vkDestroyInstance(instance, nullptr);

	// Destroy window
#ifdef GLFW
	glfwDestroyWindow(window);
#endif
}

#endif

#if DIRECTX12
#include "common.h"
#include <Windows.h>
#include "D3D12/d3d.h"

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 texCoord;
};

D3DDebug createD3DDebugInterface()
{
	D3DDebug debugInterface;
	D3DCHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
	
	return debugInterface;
}
DXGIDebug createDXGIDebugInterface()
{
	DXGIDebug debugInterface;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugInterface));
	debugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);

	return debugInterface;
}

Factory createFactory()
{
	u32 flags = 0;
#ifdef _DEBUG
	flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	Factory factory = nullptr;
	D3DCHECK(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)));

	return factory;
}

Adapter pickPhysicalAdapter(Factory factory, D3D_FEATURE_LEVEL minimumFeatureLevel)
{
	array<Adapter, 64> adapters;
	Adapter& adapter = adapters[0];
	bool found = false;
	for (u32 i = 0; factory->EnumAdapters1(i, &adapters[i]) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapter = adapters[i];
		DXGI_ADAPTER_DESC1 description;
		D3DCHECK(adapter->GetDesc1(&description));

		if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapter->Release();
			continue;
		}
		
		if (SUCCEEDED(D3D12CreateDevice(adapter, minimumFeatureLevel, __uuidof(ID3D12Device5), nullptr)))
		{
			found = true;
			break;
		}
		adapter->Release();		
	}

	assert(found);
	return adapter;
}

Device createDevice(Factory factory, Adapter adapter, D3D_FEATURE_LEVEL minimumFeatureLevel)
{
	Device device = nullptr;
	D3DCHECK(D3D12CreateDevice(adapter, minimumFeatureLevel, IID_PPV_ARGS(&device)));
	
	return device;
}

CommandQueue createCommandQueue(Device device)
{
	D3D12_COMMAND_QUEUE_DESC description;
	description.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	description.Priority = 0;
	description.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	description.NodeMask = 0;

	CommandQueue commandQueue = nullptr;
	D3DCHECK(device->CreateCommandQueue(&description, IID_PPV_ARGS(&commandQueue)));

	return commandQueue;
}

Swapchain createSwapchain(Factory factory, CommandQueue commandQueue, HWND window, int width, int height, u32 frameCount)
{
	DXGI_SWAP_CHAIN_DESC1 description;
	description.Width = width;
	description.Height = height;
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.Stereo = false;
	description.SampleDesc.Count = 1;
	description.SampleDesc.Quality = 0;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.BufferCount = frameCount;
	description.Scaling = DXGI_SCALING_STRETCH;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	description.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	description.Flags = 0;

	Swapchain swapchain = nullptr;
	HRESULT swapchainCreation = factory->CreateSwapChainForHwnd(commandQueue, window, &description, nullptr, nullptr, (IDXGISwapChain1**)&swapchain);
	D3DCHECK(swapchainCreation);

	HRESULT swapchainAssociation = factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);
	D3DCHECK(swapchainAssociation);
	return swapchain;
}

DescriptorHeap createRenderTargetViewDescriptorHeap(Device device, u32 frameCount)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescriptor;
	rtvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDescriptor.NumDescriptors = frameCount;
	rtvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDescriptor.NodeMask = 0;

	DescriptorHeap rtvDescriptorHeap = nullptr;
	D3DCHECK(device->CreateDescriptorHeap(&rtvHeapDescriptor, IID_PPV_ARGS(&rtvDescriptorHeap)));

	return rtvDescriptorHeap;
}

DescriptorHeap createShaderResourceViewDescriptorHeap(Device device)
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDescriptor;
	srvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDescriptor.NumDescriptors = 1;
	srvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDescriptor.NodeMask = 0;

	DescriptorHeap srvDescriptorHeap = nullptr;
	D3DCHECK(device->CreateDescriptorHeap(&srvHeapDescriptor, IID_PPV_ARGS(&srvDescriptorHeap)));

	return srvDescriptorHeap;
}

inline u32 incrementDescriptorSize(Device device, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return device->GetDescriptorHandleIncrementSize(heapType);
}

vector<Resource> createRenderTargetViews(Device device, Swapchain swapchain, DescriptorHeap rtvDescriptorHeap, u32 frameCount, u32 rtvDescriptorSize)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE renderTargetViewCPUHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	vector<Resource> renderTargetViews(frameCount);

	for (u32 frame = 0; frame < frameCount; frame++)
	{
		D3DCHECK(swapchain->GetBuffer(frame, IID_PPV_ARGS(&renderTargetViews[frame])));
		device->CreateRenderTargetView(renderTargetViews[frame], nullptr, renderTargetViewCPUHandle);
		renderTargetViewCPUHandle.Offset(1, rtvDescriptorSize);
	}
	
	return renderTargetViews;
}

CommandAllocator createCommandAllocator(Device device, D3D12_COMMAND_LIST_TYPE commandListType)
{
	CommandAllocator commandAllocator = nullptr;
	D3DCHECK(device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator)));
	return commandAllocator;
}

#if 0
RootSignature createRootSignature(Device device)
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescriptor;
	rootSignatureDescriptor.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Blob signature = nullptr;
	Blob error = nullptr;
	
	RootSignature rootSignature = nullptr;

	D3DCHECK(D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error));
	D3DCHECK(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

	RELEASECOM(signature);
	if (error) RELEASECOM(error);

	return rootSignature;
}
#else
RootSignature createRootSignature(Device device)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData;
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescriptor;
	rootSignatureDescriptor.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Blob signature = nullptr;
	Blob error = nullptr;

	RootSignature rootSignature = nullptr;

	HRESULT serialization = D3DX12SerializeVersionedRootSignature(&rootSignatureDescriptor, featureData.HighestVersion, &signature, &error);
	D3DCHECK(serialization);
	HRESULT rootSignatureCreation = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	D3DCHECK(rootSignatureCreation);

	RELEASECOM(signature);
	assert(!error);
	if (error) RELEASECOM(error);


	return rootSignature;
}
#endif

Blob compileShader(const wchar_t* filePath, const char* shaderEntryPoint, const char* shaderTarget)
{
	u32 compileFlags = 0;
#ifdef _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	Blob shaderBlob = nullptr;

	HRESULT compilation = D3DCompileFromFile(filePath, nullptr, nullptr, shaderEntryPoint, shaderTarget, compileFlags, 0, &shaderBlob, nullptr); // TODO: maybe checkj that last nullptr for errors!
	D3DCHECK(compilation);
	assert(compilation == S_OK);
	return shaderBlob;
}

PipelineState createGraphicsPipelineState(Device device, RootSignature rootSignature, Blob vertexShader, Blob pixelShader, Blob geometryShader = nullptr, Blob domainShader = nullptr, Blob hullShader = nullptr)
{
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		// LPCSTR SemanticName; UINT SemanticIndex; DXGI_FORMAT Format; UINT InputSlot; UINT AlignedByteOffset; D3D12_INPUT_CLASSIFICATION InputSlotClass; UINT InstanceDataStepRate;
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDescriptor = {};
	graphicsPipelineStateDescriptor.pRootSignature = rootSignature;
	graphicsPipelineStateDescriptor.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
	graphicsPipelineStateDescriptor.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	//graphicsPipelineStateDescriptor.DS = domainShader ? CD3DX12_SHADER_BYTECODE(domainShader) : D3D12_SHADER_BYTECODE{ nullptr, 0 };
	//graphicsPipelineStateDescriptor.HS = hullShader ? CD3DX12_SHADER_BYTECODE(hullShader) : D3D12_SHADER_BYTECODE{ nullptr, 0 };
	//graphicsPipelineStateDescriptor.GS = geometryShader ? CD3DX12_SHADER_BYTECODE(geometryShader) : D3D12_SHADER_BYTECODE{ nullptr, 0 };
	//graphicsPipelineStateDescriptor.StreamOutput = { nullptr, 0, nullptr, 0, 0 };
	graphicsPipelineStateDescriptor.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDescriptor.SampleMask = UINT_MAX;
	graphicsPipelineStateDescriptor.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDescriptor.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	graphicsPipelineStateDescriptor.DepthStencilState.DepthEnable = false;
	graphicsPipelineStateDescriptor.DepthStencilState.StencilEnable = false;
	graphicsPipelineStateDescriptor.InputLayout = { inputElementDescs, _countof(inputElementDescs)};
	//graphicsPipelineStateDescriptor.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	graphicsPipelineStateDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDescriptor.NumRenderTargets = 1;
	graphicsPipelineStateDescriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	//graphicsPipelineStateDescriptor.DSVFormat = DXGI_FORMAT_UNKNOWN;
	graphicsPipelineStateDescriptor.SampleDesc.Count = 1;
	//graphicsPipelineStateDescriptor.SampleDesc.Quality = 0;
	//graphicsPipelineStateDescriptor.NodeMask = 0;
	//graphicsPipelineStateDescriptor.CachedPSO = { nullptr, 0 };
	//graphicsPipelineStateDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	PipelineState graphicsPipelineState = nullptr;
	HRESULT pipelineStateCreation = device->CreateGraphicsPipelineState(&graphicsPipelineStateDescriptor, IID_PPV_ARGS(&graphicsPipelineState));
	D3DCHECK(pipelineStateCreation);

	return graphicsPipelineState;
}

Resource createVertexBuffer(Device device, const Vertex* vertices, u64 vertexBufferSize)
{
	Resource vertexBuffer = nullptr;
	D3DCHECK(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer)));

	u8* pVertexDataBegin = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	D3DCHECK(vertexBuffer->Map(0, &readRange, (void**)(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, vertices, vertexBufferSize);
	vertexBuffer->Unmap(0, nullptr);

	return vertexBuffer;
}

D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(Resource vertexBuffer, u64 vertexBufferSize)
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vertexBufferSize;
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	return vertexBufferView;
}

Resource createTexture(Device device, u64 textureWidth, u64 textureHeight)
{
	D3D12_RESOURCE_DESC textureDescriptor;
	textureDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDescriptor.Alignment = 0;
	textureDescriptor.Width = textureWidth;
	textureDescriptor.Height = textureHeight;
	textureDescriptor.DepthOrArraySize = 1;
	textureDescriptor.MipLevels = 0;
	textureDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDescriptor.SampleDesc.Count = 1;
	textureDescriptor.SampleDesc.Quality = 0;
	textureDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;


	Resource texture = nullptr;
	HRESULT textureCreation = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDescriptor,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture));
	D3DCHECK(textureCreation);

	return texture;
}

Resource uploadHeap(Device device, Resource texture)
{
	const u64 uploadBufferSize = GetRequiredIntermediateSize(texture, 0, 1);
	Resource textureUploadHeap = nullptr;
	HRESULT uploadHeapCreation = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap));

	D3DCHECK(uploadHeapCreation);

	return textureUploadHeap;
}

vector<u8> generateTexture(u64 textureWidth, u64 textureHeight, u64 texturePixelSize)
{
	const u32 rowPitch = textureWidth * texturePixelSize;
	const u32 cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
	const u32 cellHeight = textureWidth >> 3;    // The height of a cell in the checkerboard texture.
	const u32 textureSize = rowPitch * textureHeight;

	vector<u8> data(textureSize);
	u8* pData = &data[0];

	for (u32 n = 0; n < textureSize; n+= texturePixelSize)
	{
		u32 x = n % rowPitch;
		u32 y = n / rowPitch;
		u32 i = x / cellPitch;
		u32 j = y / cellHeight;

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;        // R
			pData[n + 1] = 0x00;    // G
			pData[n + 2] = 0x00;    // B
			pData[n + 3] = 0xff;    // A
		}
		else
		{
			pData[n] = 0xff;        // R
			pData[n + 1] = 0xff;    // G
			pData[n + 2] = 0xff;    // B
			pData[n + 3] = 0xff;    // A
		}
	}

	return data;
}

void updateTexture(GraphicsCommandList commandList, Resource textureResource, Resource textureUploadHeap, const vector<u8>& texture, u64 textureWidth, u64 textureHeight, u64 texturePixelSize)
{
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &texture[0];
	textureData.RowPitch = textureWidth * texturePixelSize;
	textureData.SlicePitch = textureData.RowPitch * textureHeight;
	UpdateSubresources(commandList, textureResource, textureUploadHeap, 0, 0, 1, &textureData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

Resource createShaderResourceView(Device device, DescriptorHeap srvHeap)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor = {};
	shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResourceViewDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDescriptor.Texture2D.MipLevels = 1;

	Resource shaderResourceView = nullptr;
	device->CreateShaderResourceView(shaderResourceView, &shaderResourceViewDescriptor, srvHeap->GetCPUDescriptorHandleForHeapStart());

	return shaderResourceView;
}

Fence createFence(Device device)
{
	Fence fence = nullptr;
	D3DCHECK(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	return fence;
}

HANDLE createFenceEvent()
{
	HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr)
	{
		HRESULT fenceEventError = HRESULT_FROM_WIN32(GetLastError());
		D3DCHECK(fenceEventError);
	}

	return fenceEvent;
}

u32 sync(Swapchain swapchain, CommandQueue commandQueue, Fence fence, u64& fenceValue, HANDLE fenceEvent)
{
	u64 l_fenceValue = fenceValue;
	D3DCHECK(commandQueue->Signal(fence, l_fenceValue));
	fenceValue++;

	if (fence->GetCompletedValue() < l_fenceValue)
	{
		D3DCHECK(fence->SetEventOnCompletion(l_fenceValue, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	return swapchain->GetCurrentBackBufferIndex();
}

D3D12_VIEWPORT createViewport(u32 width, u32 height)
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = float(width);
	viewport.Height = float(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	return viewport;
}
D3D12_RECT createScissorRect(u32 width, u32 height)
{
	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right =  width;
	scissorRect.bottom = height;

	return scissorRect;
}

inline D3D12_CPU_DESCRIPTOR_HANDLE getCPUDescriptorHandleForHeapStart(DescriptorHeap rtvHeap)
{
	return rtvHeap->GetCPUDescriptorHandleForHeapStart();
}

inline D3D12_GPU_DESCRIPTOR_HANDLE getGPUDescriptorHandleForHeapStart(DescriptorHeap rtvHeap)
{
	return rtvHeap->GetGPUDescriptorHandleForHeapStart();
}

void populateCommandList(CommandAllocator commandAllocator, GraphicsCommandList commandList, DescriptorHeap srvHeap, PipelineState pipelineState, RootSignature rootSignature, DescriptorHeap rtvHeap, u32 descriptorIncrementSize, const Resource* renderTargetViews, u32 frameIndex, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect)
{
	D3DCHECK(commandAllocator->Reset());

	D3DCHECK(commandList->Reset(commandAllocator, pipelineState));

	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->SetDescriptorHeaps(1, &srvHeap);
	commandList->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	// Barrier
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetViews[frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(getCPUDescriptorHandleForHeapStart(rtvHeap), frameIndex, descriptorIncrementSize);
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);
	
	// End barrier
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetViews[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	D3DCHECK(commandList->Close());
}

struct RenderStruct
{
	CommandAllocator commandAllocator;
	GraphicsCommandList commandList;
	CommandQueue commandQueue;
	Swapchain swapchain;
	Fence fence;
	u64 fenceValue;
	HANDLE fenceEvent;
	PipelineState pipelineState;
	RootSignature rootSignature;
	DescriptorHeap rtvHeap;
	DescriptorHeap srvHeap;
	u32 descriptorIncrementSize;
	const Resource* renderTargetViews;
	u32 frameIndex;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	u32 ready;
};

void RenderFrame(RenderStruct* onRender)
{
	populateCommandList(onRender->commandAllocator, onRender->commandList, onRender->srvHeap, onRender->pipelineState, onRender->rootSignature, onRender->rtvHeap, onRender->descriptorIncrementSize, onRender->renderTargetViews, onRender->frameIndex, onRender->vertexBufferView, onRender->viewport, onRender->scissorRect);

	onRender->commandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)onRender->commandList);
	HRESULT swapchainPresent = onRender->swapchain->Present(1, 0);
	D3DCHECK(swapchainPresent);

	sync(onRender->swapchain, onRender->commandQueue, onRender->fence, onRender->fenceValue, onRender->fenceEvent);
}

inline HWND win32_createWindow
(
	HINSTANCE instance,
#ifdef UNICODE
	const wchar_t* windowName,
#else
	const char* windowName,
#endif

#ifdef UNICODE
	const wchar_t* windowTitle,
#else
	const char* windowTitle,
#endif
	WNDPROC windowMessageCallback,
	bool fullscreen,
	int width, int height,
	RenderStruct* onRender
)
{
#if 0
	WNDCLASS windowClass;

	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowMessageCallback;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = instance;
	windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = windowName;

	ATOM classRegistered = RegisterClass(&windowClass);
	assert(classRegistered);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);


	if (fullscreen)
	{
		DEVMODE dmScreenSettings = {};
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (width != screenWidth && height != screenHeight)
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Full-screen mode not supported!\nSwitch to window mode", "ERROR", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					fullscreen = false;
				else
					return nullptr;
			}
		}
	}

	DWORD dwExStyle; // TODO: not used atm
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRectangle;
	windowRectangle.left = 0L;
	windowRectangle.top = 0L;
	windowRectangle.right = fullscreen ? (long)screenWidth : (long)width;
	windowRectangle.bottom = fullscreen ? (long)screenHeight : (long)height;

	BOOL windowRectAdjusted = AdjustWindowRect(&windowRectangle, dwStyle, false);
	assert(windowRectAdjusted);

	HWND windowHandle = CreateWindow(windowName, windowTitle, dwStyle, 0, 0, windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top, nullptr, nullptr, instance, onRender);
	assert(windowHandle);

	if (!fullscreen)
	{
		UINT32 x = (GetSystemMetrics(SM_CXSCREEN) - windowRectangle.right) / 2;
		UINT32 y = (GetSystemMetrics(SM_CYSCREEN) - windowRectangle.bottom) / 2;
		SetWindowPos(windowHandle, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	//ShowWindow(windowHandle, SW_SHOW);
	//SetForegroundWindow(windowHandle);
	//SetFocus(windowHandle);

#endif

	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowMessageCallback;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "Window";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, width, height };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	HWND windowHandle = CreateWindow(
		windowClass.lpszClassName,
		"Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		instance,
		onRender);

	return windowHandle;
}

LRESULT CALLBACK win32_processMessages(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool ready2 = false;
	RenderStruct* onRender = (RenderStruct*)(GetWindowLongPtr(window, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)(lParam);
		SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)(pCreateStruct->lpCreateParams));
		ready2 = true;
		return 0;
	}
	case WM_PAINT:
	{
		if (onRender && onRender->ready == 1 && ready2)
		{
			RenderFrame(onRender);
		}
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	default:
		return DefWindowProc(window, message, wParam, lParam);
	}
}

#define WIDTH 1024
#define HEIGHT 576
#define FRAMECOUNT 2
#define SHADER_SOURCE_PATH L"../shaders/"
#define SHADER_SOURCE_FILE L"shaders.hlsl"

#define SHADER_FULL_PATH WSTRING_CONCAT(SHADER_SOURCE_PATH, SHADER_SOURCE_FILE)

#include "glfw.h"

WNDPROC windowCallback = win32_processMessages;
int main(int argc, char* argv[])
// int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR, int)
{
	D3DDebug d3dDebugInterface = createD3DDebugInterface();
	DXGIDebug dxgiDebugInterface = createDXGIDebugInterface();

	Factory factory = createFactory();
	Adapter adapter = pickPhysicalAdapter(factory, minimumFeatureLevel);
	Device device = createDevice(factory, adapter, minimumFeatureLevel);
	CommandQueue commandQueue = createCommandQueue(device);
#if 0
	HINSTANCE m_instance = GetModuleHandle(nullptr);
#endif

	RenderStruct onRender;
#if 0
	HWND window = win32_createWindow(m_instance, "D3D12 Engine", "Engine Window", windowCallback, false, WIDTH, HEIGHT, &onRender);
#else
	glfwInit();
	GLFWwindow* window_glfw = glfwCreateWindow(WIDTH, HEIGHT, "Engine Window", nullptr, nullptr);
	HWND window = glfwGetWin32Window(window_glfw);
#endif

	Swapchain swapchain = createSwapchain(factory, commandQueue, window, WIDTH, HEIGHT, FRAMECOUNT);
	u32 currentBackbufferIndex = swapchain->GetCurrentBackBufferIndex();

	DescriptorHeap rtvDescriptorHeap = createRenderTargetViewDescriptorHeap(device, FRAMECOUNT);
	u32 rtvDescriptorSize = incrementDescriptorSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DescriptorHeap srvDescriptorHeap = createShaderResourceViewDescriptorHeap(device);
	vector<Resource> renderTargetViews = createRenderTargetViews(device, swapchain, rtvDescriptorHeap, FRAMECOUNT, rtvDescriptorSize);

	CommandAllocator commandAllocator = createCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	RootSignature rootSignature = createRootSignature(device);

	Blob vertexShader = compileShader(SHADER_FULL_PATH, "VSMain", "vs_5_0");
	Blob pixelShader = compileShader(SHADER_FULL_PATH, "PSMain", "ps_5_0");

	PipelineState graphicsPipelineState = createGraphicsPipelineState(device, rootSignature, vertexShader, pixelShader);

	GraphicsCommandList commandList = nullptr;
	D3DCHECK(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList)));

	constexpr float aspectRatio = FLOAT(WIDTH) / FLOAT(HEIGHT);

	Vertex triangleVertices[] =
	{
		{ { 0.0f, 0.25f * aspectRatio, 0.0f }, { 0.5f, 0.0f } },
		{ { 0.25f, -0.25f * aspectRatio, 0.0f }, { 1.0f, 1.0f } },
		{ { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f} }
	};
	constexpr const auto vertexBufferSize = sizeof(triangleVertices);
	Resource vertexBuffer = createVertexBuffer(device, triangleVertices, vertexBufferSize);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = createVertexBufferView(vertexBuffer, vertexBufferSize);

	u64 textureWidth = 256; u64 textureHeight = 256; u64 texturePixelSize = 4;
	Resource texture = createTexture(device, textureWidth, textureHeight);
	Resource textureUploadHeap = uploadHeap(device, texture);
	vector<u8> textureData = generateTexture(textureWidth, textureHeight, texturePixelSize);
	updateTexture(commandList, texture, textureUploadHeap, textureData, textureWidth, textureHeight, texturePixelSize);
	Resource shaderResourceView = createShaderResourceView(device, srvDescriptorHeap);

	D3D12_VIEWPORT viewport = createViewport(WIDTH, HEIGHT);
	D3D12_RECT scissorRect = createScissorRect(WIDTH, HEIGHT);

	D3DCHECK(commandList->Close());
	commandQueue->ExecuteCommandLists(1, (ID3D12CommandList * *)& commandList);

	Fence fence = createFence(device);
	u64 fenceValue = 1;
	HANDLE fenceEvent = createFenceEvent();

	currentBackbufferIndex = sync(swapchain, commandQueue, fence, fenceValue, fenceEvent);

	MSG msg;
	bool quitMessageReceived = false;
	
	onRender.commandAllocator = commandAllocator;
	onRender.commandList = commandList;
	onRender.commandQueue = commandQueue;
	onRender.swapchain = swapchain;
	onRender.fence = fence;
	onRender.fenceValue = fenceValue;
	onRender.fenceEvent = fenceEvent;
	onRender.pipelineState = graphicsPipelineState;
	onRender.rootSignature = rootSignature;
	onRender.rtvHeap = rtvDescriptorHeap;
	onRender.srvHeap = srvDescriptorHeap;
	onRender.descriptorIncrementSize = rtvDescriptorSize;
	onRender.renderTargetViews = renderTargetViews.data();
	onRender.frameIndex = currentBackbufferIndex;
	onRender.vertexBufferView = vertexBufferView;
	onRender.viewport = viewport;
	onRender.scissorRect = scissorRect;
	onRender.ready = 1;

#if 0
	ShowWindow(window, SW_SHOW);
#endif
#if 0
	while (!quitMessageReceived)
	{
		while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				quitMessageReceived = true;
				break;
			}
		}

		if (!IsIconic(window))
		{
			// Update and render
		}
	}
#else
	while (!glfwWindowShouldClose(window_glfw))
	{
		glfwPollEvents();

		RenderFrame(&onRender);
	}
#endif
	RELEASECOM(fence);
	RELEASECOM(vertexBuffer);
	RELEASECOM(commandList);
	//RELEASECOM(graphicsPipelineState);
	RELEASECOM(vertexShader);
	RELEASECOM(pixelShader);
	RELEASECOM(rootSignature);
	for (Resource& rt : renderTargetViews)
	{
		RELEASECOM(rt);
	}

	RELEASECOM(rtvDescriptorHeap);
	RELEASECOM(swapchain);
	RELEASECOM(commandQueue);
	RELEASECOM(device);
	RELEASECOM(adapter);
	RELEASECOM(factory);
	RELEASECOM(d3dDebugInterface);
	RELEASECOM(dxgiDebugInterface);
}

#endif

#if OPENGL
#define WIDTH 1024
#define HEIGHT 576
#include "common.h"
#define GLFW_INCLUDE_NONE
#include "glfw.h"
#include "OPENGL/opengl.h"

int main(int argumentCount, char** argumentValues)
{
	GLFWwindow* window = nullptr;

	auto initSuccess = glfwInit();
	assert(initSuccess);

	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Engine", nullptr, nullptr);
	assert(window);

	glfwMakeContextCurrent(window);

	auto gladInitSucess = gladLoadGL();
	assert(gladInitSucess);

	printf("%s\n", glGetString(GL_VERSION));
	system("PAUSE");

	//while (!glfwWindowShouldClose(window))
	//{
	//	glClear(GL_COLOR_BUFFER_BIT);

	//	glBegin(GL_TRIANGLES);

	//	glVertex2f(-0.5f, -0.5f);
	//	glVertex2f(0.0f, 0.5f);
	//	glVertex2f(0.5f, -0.5f);

	//	glEnd();
	//	glfwSwapBuffers(window);
	//	glfwPollEvents();
	//}

	glfwTerminate();
}
#endif

#if DIRECTX11
#include "D3D11/d3d.h"
#define WIDTH 1024
#define HEIGHT 576
int main(int argumentCount, char** argumentValues)
{
	const int width = WIDTH;
	const int height = HEIGHT;
	HINSTANCE currentInstance = GetModuleHandle(nullptr);
	HWND window = createWindow(currentInstance, DefWindowProc, width, height, "D3D11 Engine");
	ShowWindow(window, SW_SHOW);
	
	Device _device = createDevice(D3D_DRIVER_TYPE_HARDWARE);
	ID3D11Device* device = _device.device;
	ID3D11DeviceContext* deviceContext = _device.context;
	D3D_FEATURE_LEVEL featureLevel = _device.featureLevel;

	u32 multisamplingLevel = 4;
	bool multisamplingSupported = checkForMultisampling(device, DXGI_FORMAT_B8G8R8A8_UNORM, multisamplingLevel);
	// Multisampling doesn't work at the moment: not enabling it!
	IDXGISwapChain* swapchain = createSwapchain(device, width, height, multisamplingSupported, multisamplingLevel, window);
	ID3D11RenderTargetView* renderTargetView = createRenderTargetView(device, swapchain, 0);
	ID3D11Texture2D* depthStencilBuffer = createDepthStencilBuffer(device, width, height);
	ID3D11DepthStencilView* depthStencilView = createDepthStencilView(device, depthStencilBuffer);
	D3D11_VIEWPORT viewport = createViewport(width, height);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_PAINT)
			{
				setRenderTargets(deviceContext, 1, &renderTargetView, depthStencilView);
				deviceContext->ClearRenderTargetView(renderTargetView, (const float*) (&Colors::Blue));
				deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

				D3DCHECK(swapchain->Present(0, 0));
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}
#endif