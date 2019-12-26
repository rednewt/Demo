#include "pch.h"
#include "Timer.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool InitializeWindow(HINSTANCE hInstance, HWND& outWindowHandle);

const int g_ClientWidth = 1600;
const int g_ClientHeight = 900;

INT WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd)
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HWND windowHandle;
	if (!InitializeWindow(hInstance, windowHandle))
	{
		MessageBox(NULL, L"Failed to create a window", 0, MB_OK | MB_ICONERROR);
		return -1;
	}

	ShowWindow(windowHandle, nShowCmd);
	UpdateWindow(windowHandle);

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			
		}
	}

	return static_cast<int>(msg.wParam);
}

bool InitializeWindow(HINSTANCE hInstance, HWND& windowHandle)
{
	WNDCLASS wc = {};

	const std::wstring WindowClass = L"WindowClass1";

	wc.hInstance = hInstance;
	wc.lpszClassName = WindowClass.c_str();
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;

	if (!RegisterClass(&wc))
		return false;

	RECT wr = { 0, 0, g_ClientWidth, g_ClientHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	windowHandle = CreateWindow(WindowClass.c_str(), L"Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		0, 0, hInstance, 0);

	if (!windowHandle)
		return false;
	
	
	return true;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	switch (Msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}


	return DefWindowProc(hWnd, Msg, wParam, lParam);
}