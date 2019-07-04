#pragma once
#include "common.h"
#include <windows.h>
#include <d3d11.h>
#include <d3d11_4.h>
#include <DirectXColors.h>
#include <comdef.h>
using namespace DirectX;

inline void DxError(HRESULT hr, const char* function, const char* file, int line)
{
	_com_error error(hr);
	fprintf(stderr, "DIRECTX ERROR\n-------------\n%s failed!\nFILE: %s\nLINE: %d\nERROR MESSAGE: %s\n\n", function, file, line, error.ErrorMessage());
}

#define D3DCHECK(result)								\
{														\
	HRESULT _result = result;							\
	if (FAILED(_result)) DxError(_result, #result, __FILE__, __LINE__);	\
}

#define RELEASECOM(comPtr)												\
{																		\
	comPtr->Release();													\
	comPtr = nullptr;													\
}

HWND createWindow(HINSTANCE instance, WNDPROC windowProc, int width, int height, const char* windowTitle, void* dataPointer = nullptr)
{
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "Window Class";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, width, height};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	HWND myWindow = CreateWindow(
		windowClass.lpszClassName,
		"Window Title",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		instance,
		dataPointer);

	return myWindow;
}

struct Device
{
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	D3D_FEATURE_LEVEL featureLevel;
};

Device createDevice(D3D_DRIVER_TYPE driverType)
{
	u32 createDeviceFlags = 0;
#if _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	Device device;
	D3DCHECK(D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &device.device, &device.featureLevel, &device.context));

	return device;
}

bool checkForMultisampling(ID3D11Device* device, DXGI_FORMAT multisamplingFormat, u32 multisamplingLevel)
{
	u32 MSAA_quality = 0;
	D3DCHECK(device->CheckMultisampleQualityLevels(multisamplingFormat, multisamplingLevel, &MSAA_quality));
	return MSAA_quality > 0;
}

IDXGISwapChain* createSwapchain(ID3D11Device* device, int width, int height, bool multisamplingSupported, u32 multisamplingQuality, HWND window)
{
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	/*sd.SampleDesc.Count = multisamplingSupported ? 4 : 1;
	sd.SampleDesc.Quality = multisamplingSupported ? (multisamplingQuality - 1) : 0;*/
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = window;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = nullptr;
	D3DCHECK(device->QueryInterface(__uuidof(IDXGIDevice), (void**)& dxgiDevice));

	IDXGIAdapter* dxgiAdapter = nullptr;
	D3DCHECK(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)& dxgiAdapter));

	IDXGIFactory* dxgiFactory = nullptr;
	D3DCHECK(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)& dxgiFactory));

	IDXGISwapChain* swapchain = nullptr;
	D3DCHECK(dxgiFactory->CreateSwapChain(device, &sd, &swapchain));

	RELEASECOM(dxgiDevice);
	RELEASECOM(dxgiAdapter);
	RELEASECOM(dxgiFactory);

	return swapchain;
}

ID3D11RenderTargetView* createRenderTargetView(ID3D11Device* device, IDXGISwapChain* swapchain, u32 frameIndex)
{
	ID3D11Texture2D* backbuffer;
	D3DCHECK(swapchain->GetBuffer(frameIndex, IID_PPV_ARGS(&backbuffer)));
	ID3D11RenderTargetView* renderTargetView = nullptr;
	D3DCHECK(device->CreateRenderTargetView(backbuffer, nullptr, &renderTargetView));
	RELEASECOM(backbuffer);

	return renderTargetView;
}

ID3D11Texture2D* createDepthStencilBuffer(ID3D11Device* device, int width, int height)
{
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ID3D11Texture2D* depthStencilBuffer = nullptr;
	D3DCHECK(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer));

	return depthStencilBuffer;
}

ID3D11DepthStencilView* createDepthStencilView(ID3D11Device* device, ID3D11Texture2D* depthStencilBuffer)
{
	ID3D11DepthStencilView* depthStencilView = nullptr;
	D3DCHECK(device->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView));
	return depthStencilView;
}

D3D11_VIEWPORT createViewport(int width, int height)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = float(width);
	viewport.Height = float(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	return viewport;
}

void setRenderTargets(ID3D11DeviceContext* deviceContext, u32 renderTargetCount, ID3D11RenderTargetView* const* renderTargetViews, ID3D11DepthStencilView* depthStencilView)
{
	deviceContext->OMSetRenderTargets(renderTargetCount, renderTargetViews, depthStencilView);
}
//void onResize(ID3D11Device* device, IDXGISwapChain* swapchain, DXGI_FORMAT swapchainFormat, u32 nRenderTargets, int width, int height)
//{
//	D3DCHECK(swapchain->ResizeBuffers(nRenderTargets, width, height, swapchainFormat, 0));
//	createRenderTargetView
//	ReleaseCOM(backBuffer);
//
//	// Create the depth/stencil buffer and view.
//
//	D3D11_TEXTURE2D_DESC depthStencilDesc;
//
//	depthStencilDesc.Width = mClientWidth;
//	depthStencilDesc.Height = mClientHeight;
//	depthStencilDesc.MipLevels = 1;
//	depthStencilDesc.ArraySize = 1;
//	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//
//	// Use 4X MSAA? --must match swap chain MSAA values.
//	if (mEnable4xMsaa)
//	{
//		depthStencilDesc.SampleDesc.Count = 4;
//		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
//	}
//	// No MSAA
//	else
//	{
//		depthStencilDesc.SampleDesc.Count = 1;
//		depthStencilDesc.SampleDesc.Quality = 0;
//	}
//
//	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
//	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//	depthStencilDesc.CPUAccessFlags = 0;
//	depthStencilDesc.MiscFlags = 0;
//
//	D3DCHECK(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
//	D3DCHECK(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));
//
//
//	// Bind the render target view and depth/stencil view to the pipeline.
//
//	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
//
//
//	// Set the viewport transform.
//
//	mScreenViewport.TopLeftX = 0;
//	mScreenViewport.TopLeftY = 0;
//	mScreenViewport.Width = static_cast<float>(mClientWidth);
//	mScreenViewport.Height = static_cast<float>(mClientHeight);
//	mScreenViewport.MinDepth = 0.0f;
//	mScreenViewport.MaxDepth = 1.0f;
//
//	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
//}