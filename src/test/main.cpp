//--------------------------------------------------------------------------------------
// File: Tutorial06.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include "core/rendersys/render_system.h"
#include "test/app.h"

#define C_WINDOW_WIDTH 1024
#define C_WINDOW_HEIGHT 768

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, const char* name, HWND* pHandle);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char* ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int argc = 0; 
	LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);

	std::string appName = GetCurrentAppName();
	int caseIndex = -1;
	if (argc > 1) {
		appName = ConvertLPWSTRToLPSTR(argv[1]);
		if (argc > 2) caseIndex = atoi(ConvertLPWSTRToLPSTR(argv[2]));
	}
	else {
		FILE* fd = fopen("test_cmdline.txt", "r");
		if (fd) {
			char szAppName[260];
			fscanf(fd, "%s %d", szAppName, &caseIndex);
			appName = szAppName;
			fclose(fd);
		}
	}

	auto AppDraw = CreateApp(appName);
	if (caseIndex != -1)
		AppDraw->SetCaseIndex(caseIndex);

	HWND handle;
	if (FAILED(InitWindow(hInstance, nCmdShow, AppDraw->GetName().c_str(), &handle)))
		return 0;

	if (!AppDraw->Initialize(hInstance, handle))
		return 0;

	MSG msg = { 0 };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			AppDraw->Render();
		}
	}

	AppDraw->CleanUp();
	return (int)msg.wParam;
}

char* ConvertLPWSTRToLPSTR(LPWSTR lpwszStrIn)
{
	LPSTR pszOut = NULL;
	try
	{
		if (lpwszStrIn != NULL)
		{
			int nInputStrLen = wcslen(lpwszStrIn);
			int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
			pszOut = new char[nOutputStrLen];
			if (pszOut)
			{
				memset(pszOut, 0x00, nOutputStrLen);
				WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
			}
		}
	}
	catch (std::exception e)
	{}

	return pszOut;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow, const char* name, HWND* pHandle)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)0);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)0);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	RECT rc = { 0, 0, C_WINDOW_WIDTH, C_WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	auto HWnd = CreateWindowA("TutorialWindowClass", name, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (!HWnd)
		return E_FAIL;

	*pHandle = HWnd;

	ShowWindow(HWnd, nCmdShow);
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}