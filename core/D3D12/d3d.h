#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <comdef.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

using namespace DirectX;

typedef ID3D12Debug3* D3DDebug;
typedef IDXGIDebug1* DXGIDebug;
typedef IDXGIFactory7* Factory;
typedef IDXGIAdapter1* Adapter;
typedef ID3D12Device5* Device;
typedef ID3D12CommandQueue* CommandQueue;
typedef IDXGISwapChain4* Swapchain;
typedef ID3D12DescriptorHeap* DescriptorHeap;
typedef ID3D12Resource* Resource;
typedef ID3D12CommandAllocator* CommandAllocator;
typedef ID3D12RootSignature* RootSignature;
typedef ID3DBlob* Blob;
typedef ID3D12PipelineState* PipelineState;
typedef ID3D12CommandList* CommandList;
typedef ID3D12GraphicsCommandList* GraphicsCommandList;
typedef ID3D12Fence* Fence;
const D3D_FEATURE_LEVEL minimumFeatureLevel = D3D_FEATURE_LEVEL_12_1;

#ifdef UNICODE
inline void DxError(HRESULT hr, )

#define D3DCHECK(result)								\
{														\
	HRESULT _result = result;							\
	if (FAILED(result))
}
#else
inline void DxError(HRESULT hr, const char* function, const char* file, int line)
{
	_com_error error(hr);
	fprintf(stderr, "DIRECTX ERROR\n-------------\n%s failed!\nFILE: %s\nLINE: %d\nERROR MESSAGE: %s\n\n", function, file, line, error.ErrorMessage());
}

#define D3DCHECK(result)												\
{																		\
	HRESULT _result = result;											\
	if (FAILED(_result)) DxError(_result, #result, __FILE__, __LINE__);	\
}
#endif

#define RELEASECOM(comPtr)												\
{																		\
	comPtr->Release();													\
	comPtr = nullptr;													\
}