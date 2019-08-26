#if _WIN64
#pragma once

#include "common.h"
#include <Windows.h>
#include <comdef.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

using namespace DirectX;

struct VertexFake
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

// TODO: please, refer to this for vertex layout changes!
using DirectXVertex = VertexFake;

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

#endif