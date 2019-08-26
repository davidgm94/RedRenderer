#pragma once
#include "../common.h"
#include <d3d11.h>
#include "../d3d.h"

#pragma once
#include "../common.h"
#include "d3dcommon.h"
#include <d3d11.h>
#include <d3d11shadertracing.h>
#include "../win32.h"
#include <string>
#include <math.h>

const constexpr float PI = XM_PI;

extern i64 performanceFrequency;

struct ConstantBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
};

static ConstantBuffer cb;

struct RenderedScene
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;

};

struct DepthStencil
{
	ID3D11Texture2D* buffer;
	ID3D11DepthStencilView* view;
};

struct D3D11_Renderer
{
	ID3D11Debug* debug;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapchain;
	ID3D11RenderTargetView* renderTargetView;
	DepthStencil depthStencil;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* constantBuffer;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	int width, height;
};

IDXGISwapChain* createSwapchain(ID3D11Device* device, int width, int height, HWND window)
{
	DXGI_SWAP_CHAIN_DESC swapchainDescription;

	swapchainDescription.BufferDesc.Width = width;
	swapchainDescription.BufferDesc.Height = height;
	swapchainDescription.BufferDesc.RefreshRate.Numerator = 60;
	swapchainDescription.BufferDesc.RefreshRate.Denominator = 1;
	swapchainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapchainDescription.SampleDesc.Count = 1;
	swapchainDescription.SampleDesc.Quality = 0;
	swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDescription.BufferCount = 1;
	swapchainDescription.OutputWindow = window;
	swapchainDescription.Windowed = true;
	swapchainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchainDescription.Flags = 0;

	IDXGIDevice* dxgiDevice = nullptr;
	D3DCHECK(device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)));

	IDXGIAdapter* dxgiAdapter = nullptr;
	D3DCHECK(dxgiDevice->GetParent(IID_PPV_ARGS(&dxgiAdapter)));

	IDXGIFactory* dxgiFactory = nullptr;
	D3DCHECK(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

	IDXGISwapChain* swapchain = nullptr;
	D3DCHECK(dxgiFactory->CreateSwapChain(device, &swapchainDescription, &swapchain));

	RELEASECOM(dxgiDevice);
	RELEASECOM(dxgiAdapter);
	RELEASECOM(dxgiFactory);

	return swapchain;
}

ID3D11RenderTargetView* createRenderTargetView(ID3D11Device* device, IDXGISwapChain* swapchain)
{
	ID3D11Texture2D* backbuffer;
	D3DCHECK(swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer)));
	ID3D11RenderTargetView* renderTargetView;
	D3DCHECK(device->CreateRenderTargetView(backbuffer, nullptr, &renderTargetView));

	RELEASECOM(backbuffer);

	return renderTargetView;
}

DepthStencil createDepthStencil(ID3D11Device* device, int width, int height)
{
	D3D11_TEXTURE2D_DESC depthStencilDescription;
	depthStencilDescription.Width = width;
	depthStencilDescription.Height = height;
	depthStencilDescription.MipLevels = 1;
	depthStencilDescription.ArraySize = 1;
	depthStencilDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDescription.SampleDesc.Count = 1;
	depthStencilDescription.SampleDesc.Quality = 0;
	depthStencilDescription.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDescription.CPUAccessFlags = 0;
	depthStencilDescription.MiscFlags = 0;

	DepthStencil depthStencil;
	D3DCHECK(device->CreateTexture2D(&depthStencilDescription, nullptr, &depthStencil.buffer));
	D3DCHECK(device->CreateDepthStencilView(depthStencil.buffer, 0, &depthStencil.view));

	return depthStencil;
}

inline void setRenderTargets(ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv)
{
	deviceContext->OMSetRenderTargets(1, &rtv, dsv);
}

inline void setViewport(ID3D11DeviceContext* deviceContext, int width, int height)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = float(width);
	viewport.Height = float(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	deviceContext->RSSetViewports(1, &viewport);
}

void handleResize(RECT resolution, void* APIConfig)
{
	D3D11_Renderer& renderer = *(D3D11_Renderer*)APIConfig;
	renderer.width = resolution.right;
	renderer.height = resolution.bottom;
	RELEASECOM(renderer.renderTargetView);
	RELEASECOM(renderer.depthStencil.view);
	RELEASECOM(renderer.depthStencil.buffer);

	D3DCHECK(renderer.swapchain->ResizeBuffers(1, renderer.width, renderer.height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	renderer.renderTargetView = createRenderTargetView(renderer.device, renderer.swapchain);
	renderer.depthStencil = createDepthStencil(renderer.device, renderer.width, renderer.height);
	setRenderTargets(renderer.context, renderer.renderTargetView, renderer.depthStencil.view);
	setViewport(renderer.context, renderer.width, renderer.height);
}

ID3D11Buffer* createVertexBuffer(ID3D11Device* device, const void* vertices, u32 vertexCount, u32 vertexBufferSize)
{
	assert(vertexCount * sizeof(DirectXVertex) == vertexBufferSize);

	D3D11_BUFFER_DESC vertexBufferDescription;
	vertexBufferDescription.ByteWidth = vertexBufferSize;
	vertexBufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescription.CPUAccessFlags = 0;
	vertexBufferDescription.MiscFlags = 0;
	vertexBufferDescription.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = vertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	ID3D11Buffer* vertexBuffer;
	D3DCHECK(device->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &vertexBuffer));

	return vertexBuffer;
}

ID3D11Buffer* createIndexBuffer(ID3D11Device* device, const u32* indices, u32 indexCount, u32 indexBufferSize)
{
	assert(indexBufferSize == indexCount * sizeof(u32));

	D3D11_BUFFER_DESC indexBufferDescription;
	indexBufferDescription.ByteWidth = indexBufferSize;
	indexBufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescription.CPUAccessFlags = 0;
	indexBufferDescription.MiscFlags = 0;
	indexBufferDescription.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	ID3D11Buffer* indexBuffer;
	D3DCHECK(device->CreateBuffer(&indexBufferDescription, &indexBufferData, &indexBuffer));

	return indexBuffer;
}

ID3DBlob* readCompiledShader(const wchar_t* compiledBufferFilename)
{
	ID3DBlob* shaderObjectBuffer;
	HRESULT shaderRead = D3DReadFileToBlob(compiledBufferFilename, &shaderObjectBuffer);
	D3DCHECK(shaderRead);
	return shaderObjectBuffer;
}

ID3D11DeviceChild* createShader(ID3D11Device* device, D3D11_SHADER_TYPE shaderType, ID3DBlob* bytecodeBuffer)
{
	assert(bytecodeBuffer);

	switch (shaderType)
	{
		case (D3D11_VERTEX_SHADER):
		{
			ID3D11VertexShader* vertexShader;
			D3DCHECK(device->CreateVertexShader(bytecodeBuffer->GetBufferPointer(), bytecodeBuffer->GetBufferSize(), nullptr, &vertexShader));
			// TODO: it's not freeing the blob so you can create the input layout. Be careful!!! It's freed in the create input layout function
			// warning!
			return vertexShader;
		} break;
		case (D3D11_PIXEL_SHADER):
		{
			ID3D11PixelShader* pixelShader;
			D3DCHECK(device->CreatePixelShader(bytecodeBuffer->GetBufferPointer(), bytecodeBuffer->GetBufferSize(), nullptr, &pixelShader));
			RELEASECOM(bytecodeBuffer);
			return pixelShader;
		} break;
		default:
		{
			assert(false && "Not implemented yet");
			return nullptr;
		} break;
	}
}

ID3D11InputLayout* createInputLayout(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* vertexDescription, size_t vertexDescriptionSize, ID3DBlob* vertexShaderBytecode)
{
	ID3D11InputLayout* inputLayout;
	D3DCHECK(device->CreateInputLayout(vertexDescription, vertexDescriptionSize, vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), &inputLayout));
	RELEASECOM(vertexShaderBytecode);
	return inputLayout;
}

ID3D11Buffer* createConstantBuffer(ID3D11Device* device, size_t bufferSize)
{
	D3D11_BUFFER_DESC cbDescriptor;
	cbDescriptor.ByteWidth = bufferSize;
	cbDescriptor.Usage = D3D11_USAGE_DEFAULT;
	cbDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDescriptor.CPUAccessFlags = 0;
	cbDescriptor.MiscFlags = 0;
	cbDescriptor.StructureByteStride = 0;

	ID3D11Buffer* constantBuffer;
	D3DCHECK(device->CreateBuffer(&cbDescriptor, nullptr, &constantBuffer));
	return constantBuffer;
}

void createBuffers(D3D11_Renderer& renderer)
{
	DirectXVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
	};

	u32 indices[] = {
		// front face
		3,1,0,
		2,1,3,

		0,5,4,
		1,5,0,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		2,7,6,
		3,7,2,

		6,4,5,
		7,4,6,
	};

	renderer.vertexBuffer = createVertexBuffer(renderer.device, vertices, ARRAYSIZE(vertices), sizeof(vertices));
	renderer.indexBuffer = createIndexBuffer(renderer.device, indices, ARRAYSIZE(indices), sizeof(indices));
	renderer.constantBuffer = createConstantBuffer(renderer.device, sizeof(ConstantBuffer));
}



D3D11_Renderer initDirectX11(HWND window, RECT area)
{
	D3D11_Renderer renderer;
	renderer.width = area.right;
	renderer.height = area.bottom;

	u32 createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT deviceCreation = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &renderer.device, &featureLevel, &renderer.context);
	D3DCHECK(deviceCreation);
#ifdef _DEBUG
	renderer.device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&renderer.debug));
#endif

	renderer.swapchain = createSwapchain(renderer.device, renderer.width, renderer.height, window);
	renderer.renderTargetView = createRenderTargetView(renderer.device, renderer.swapchain);
	renderer.depthStencil = createDepthStencil(renderer.device, renderer.width, renderer.height);
	setRenderTargets(renderer.context, renderer.renderTargetView, renderer.depthStencil.view);
	setViewport(renderer.context, renderer.width, renderer.height);

	// custom
	createBuffers(renderer);
	const wchar_t* vertexShaderFilename = L"../../shaders/bytecode/box.vert.cso";
	const wchar_t* pixelShaderFilename = L"../../shaders/bytecode/box.pixel.cso";
	ID3DBlob* vertexShaderBytecode = readCompiledShader(vertexShaderFilename);
	ID3DBlob* pixelShaderBytecode = readCompiledShader(pixelShaderFilename);

	//renderer.fx = loadFxShader(renderer.device, "d3d/fx/color.cso");
	renderer.vertexShader = (ID3D11VertexShader*)(createShader(renderer.device, D3D11_VERTEX_SHADER, vertexShaderBytecode));
	renderer.pixelShader = (ID3D11PixelShader*)(createShader(renderer.device, D3D11_PIXEL_SHADER, pixelShaderBytecode));

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	renderer.inputLayout = createInputLayout(renderer.device, vertexDesc, ARRAYSIZE(vertexDesc), vertexShaderBytecode);

	return renderer;
}

RenderedScene initScene()
{
	RenderedScene scene;

	// Initialize the world matrix
	scene.world = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	scene.view = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	scene.proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1024 / (FLOAT)576, 0.01f, 100.0f);

	return scene;
}

void update(RenderedScene& scene, float dt)
{
	scene.world = XMMatrixRotationX(dt);

	// Transpose!! (WHY?? TODO:)
	cb.world = XMMatrixTranspose(scene.world);
	cb.view = XMMatrixTranspose(scene.view);
	cb.proj = XMMatrixTranspose(scene.proj);
}

void render(D3D11_Renderer& renderer, RenderedScene& scene)
{
	renderer.context->ClearRenderTargetView(renderer.renderTargetView, Colors::LightBlue);
	renderer.context->ClearDepthStencilView(renderer.depthStencil.view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	renderer.context->IASetInputLayout(renderer.inputLayout);
	renderer.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	u32 stride = sizeof(DirectXVertex);
	u32 offset = 0;

	renderer.context->IASetVertexBuffers(0, 1, &renderer.vertexBuffer, &stride, &offset);
	renderer.context->IASetIndexBuffer(renderer.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	renderer.context->UpdateSubresource(renderer.constantBuffer, 0, nullptr, &cb, 0, 0);

	renderer.context->VSSetShader(renderer.vertexShader, nullptr, 0);
	renderer.context->VSSetConstantBuffers(0, 1, &renderer.constantBuffer);
	renderer.context->PSSetShader(renderer.pixelShader, nullptr, 0);

	renderer.context->DrawIndexed(36, 0, 0);

	D3DCHECK(renderer.swapchain->Present(0, 0));
}

void shutdownD3D11Renderer(D3D11_Renderer& renderer)
{
	RELEASECOM(renderer.pixelShader);
	RELEASECOM(renderer.vertexShader);
	RELEASECOM(renderer.inputLayout);
	RELEASECOM(renderer.constantBuffer);
	RELEASECOM(renderer.vertexBuffer);
	RELEASECOM(renderer.indexBuffer);
	RELEASECOM(renderer.depthStencil.view);
	RELEASECOM(renderer.depthStencil.buffer);
	RELEASECOM(renderer.renderTargetView);
	RELEASECOM(renderer.swapchain);
	RELEASECOM(renderer.context);
	RELEASECOM(renderer.device);
#ifdef _DEBUG
	//D3DCHECK(renderer.debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL));
	RELEASECOM(renderer.debug);
#endif

}