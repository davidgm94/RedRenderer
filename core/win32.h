#pragma once
#include "common.h"
#include <Windows.h>
#include <ShellScalingApi.h>

#define WIN32_HANDLE_MESSAGES_DEFAULT(window)										\
{																			\
	MSG msg;																\
	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))						\
	{																		\
		TranslateMessage(&msg);												\
		DispatchMessage(&msg);												\
	}																		\
}

void setupConsole(const char* title)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	freopen_s(&stream, "CONOUT$", "w+", stderr);
	SetConsoleTitle(TEXT(title));
}

void setupDPIAwareness()
{
	typedef HRESULT* (__stdcall * SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS);

	HMODULE shCore = LoadLibraryA("Shcore.dll");
	if (shCore)
	{
		SetProcessDpiAwarenessFunc setProcessDpiAwareness =
			(SetProcessDpiAwarenessFunc)GetProcAddress(shCore, "SetProcessDpiAwareness");

		if (setProcessDpiAwareness != nullptr)
		{
			setProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		}

		FreeLibrary(shCore);
	}
}

// Forward declarations
typedef void* APIConfigPtr;

struct Win32_WindowDimension
{
	u32 width, height;
};

struct Win32_ApplicationInfo
{
	HWND window;
	bool32 resizing;
	bool32 running;
	APIConfigPtr apiConfig;
	RED_RENDERER_GRAPHICS_API api;
	RECT clientArea;
};


i64 win32_getTimerValue()
{
	i64 performanceCounter;
	QueryPerformanceCounter((LARGE_INTEGER*)& performanceCounter);
	return performanceCounter;
}

i64 win32_getTimerFrequency()
{
	i64 performanceFrequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&performanceFrequency);
	return performanceFrequency;
}

float win32_deltaT(i64 initTime, i64 frequency)
{
	return (float)(win32_getTimerValue() - initTime) / frequency;
}


HWND win32_createWindow(HINSTANCE instance, WNDPROC windowProc, int width, int height, const char* windowTitle, void* dataPointer)
{
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbWndExtra = 64;
	windowClass.cbSize = sizeof(windowClass);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = windowProc;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "Window Class";
	RegisterClassEx(&windowClass);

	RECT windowRect = { 0, 0, width, height };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	HWND myWindow = CreateWindow(
		windowClass.lpszClassName,
		windowTitle,
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

LRESULT CALLBACK win32_messageCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32_ApplicationInfo* appInfo = ((Win32_ApplicationInfo*)GetWindowLongPtr(window, GWLP_USERDATA));
	switch (message)
	{
		case (WM_PAINT):
		{
			ValidateRect(window, nullptr);
		} break;
		case (WM_CLOSE):
		{
			if (appInfo)
			{
				appInfo->running = false;
				DestroyWindow(window);
				PostQuitMessage(0);
			}
		} break;
		case (WM_CREATE):
		{
			LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)(lParam);
			SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)(pCreateStruct->lpCreateParams));
		} break;
		default:
		{

		} break;
	}
	return DefWindowProc(window, message, wParam, lParam);
}